//
// Created by Ronald Rahaman on 2019-01-23.
//

#include "NearestPointReceiver.h"
#include "AuxiliarySystem.h"

#include "mpi.h"
#include "OpenMCProblem.h"
#include "openmc/capi.h"
#include "openmc/cell.h"
#include "openmc/constants.h"
#include "openmc/particle.h"
#include "openmc/geometry.h"
#include "openmc/message_passing.h"
#include "openmc/settings.h"
#include "openmc/summary.h"
#include "xtensor/xarray.hpp"
#include "xtensor/xview.hpp"

registerMooseObject("OpenMCApp", OpenMCProblem);

template<>
InputParameters
validParams<OpenMCProblem>()
{
  InputParameters params = validParams<ExternalProblem>();
  params.addRequiredParam<Real>("power", "specified power for OpenMC");
  params.addRequiredParam<std::vector<Point>>("centers", "coords of pebble centers");
  params.addRequiredParam<std::vector<Real>>("volumes", "volumes of pebbles");
  params.addRequiredParam<std::string>("tally_type", "type of tally to use in OpenMC");
  params.addParam<std::string>("mesh_template", "mesh tally template for OpenMC");
  return params;
}

OpenMCProblem::OpenMCProblem(const InputParameters &params) :
  ExternalProblem(params),
  _centers(getParam<std::vector<Point>>("centers")),
  _power(getParam<Real>("power")),
  _volumes(getParam<std::vector<Real>>("volumes"))
{

  int err;

  auto tallyTypeStr = getParam<std::string>("tally_type");

  if (tallyTypeStr == "cell") {
    _tallyType = TallyType::CELL;
  } else if (tallyTypeStr == "mesh") {
    _tallyType = TallyType::MESH;
  } else {
    mooseError("Invalid tally type specified: " + tallyTypeStr);
  }

  if (_tallyType == TallyType::MESH) {
    _meshTemplateFilename = getParam<std::string>("mesh_template");
  }

  auto& m = mesh().getMesh();

  if (openmc::settings::libmesh_comm) {
    mooseWarning("libMesh communicator already set in OpenMC.");
  }

  openmc::settings::libmesh_comm = &m.comm();

    // Find cell for each pebble center
    // _centers is initialized with the pebble centers from .i file
    for (auto &c : _centers) {
      openmc::Particle p {};
      p.r() = {c(0), c(1), c(2)};
      p.u() = {0., 0., 1.};
      openmc::find_cell(p, false);
      _cellIndices.push_back(p.coord_[0].cell);
      _cellInstances.push_back(p.cell_instance_);
    }

  if (_tallyType == TallyType::CELL) {
    setupCellTally();
  } else if (_tallyType == TallyType::MESH) {
    setupMeshTallies();
  }
}

void OpenMCProblem::setupCellTally() {
  // create a cell filter
  auto cellFilter = dynamic_cast<openmc::CellFilter*>(openmc::Filter::create("cell"));
  _cellFilters.push_back(cellFilter);
  cellFilter->set_cells(_cellIndices);

  // create a new tally
  auto tally = openmc::Tally::create();
  _tallies.push_back(tally);

  tally->set_id(-1);
  std::vector<openmc::Filter*> tally_filters = {cellFilter};
  tally->set_filters(tally_filters);
  tally->set_scores({"kappa-fission"});
}

void OpenMCProblem::setupMeshTallies() {
  if (_meshTemplateFilename.empty()) {
    mooseError("No template filename specified.");
  }

  // create the OpenMC mesh instance
  int meshId = 0;
  for (const auto& mesh : openmc::model::meshes) { meshId = std::max(meshId, mesh->id_); }
  auto mesh = std::make_unique<openmc::LibMesh>(_meshTemplateFilename);
  mesh->id_ = ++meshId;
  mesh->output_ = false;

  _meshTemplate = mesh.get();

  int32_t mesh_index = openmc::model::meshes.size();
  openmc::model::mesh_map[mesh->id_] = mesh_index;
  openmc::model::meshes.push_back(std::move(mesh));

  // setup a new mesh filter
  int32_t meshFilterId;
  openmc_get_filter_next_id(&meshFilterId);

  int32_t meshTallyId;
  openmc_get_tally_next_id(&meshTallyId);

  for (auto& c : _centers) {
    auto meshFilter = dynamic_cast<openmc::MeshFilter*>(openmc::Filter::create("mesh"));
    meshFilter->set_mesh(mesh_index);
    meshFilter->set_translation({c(0), c(1), c(2)});

    _meshFilters.push_back(meshFilter);

    std::vector<openmc::Filter*> tally_filters = {meshFilter};

    // apply the mesh filter to a tally
    auto tally = openmc::Tally::create();
    tally->set_filters(tally_filters);
    tally->set_scores({"kappa-fission"});
    tally->estimator_ = openmc::TallyEstimator::COLLISION;
    _tallies.push_back(tally);
  }


  // performance optimization - assume the mesh tallies are spatially separate
  openmc::settings::assume_separate = true;

  // warn user that this setting is present if other tallies existin
  // (in the tallies.xml for example)
  if (openmc::model::tallies.size() > _tallies.size()) {
    mooseWarning("Additional tallies exist in the problem. "
                 "If spatial overlaps exist, tally data may be inaccurate");
  }
}

void OpenMCProblem::addExternalVariables()
{
  FEType element(CONSTANT, MONOMIAL);

  addAuxVariable("heat_source", element);
  _heat_source_var = _aux->getFieldVariable<Real>(0, "heat_source").number();

    auto receiver_params = _factory.getValidParams("NearestPointReceiver");
    receiver_params.set<std::vector<Point>>("positions") = _centers;

    // Will receive values from the master
    addUserObject("NearestPointReceiver", "average_temp", receiver_params);

    // Initialize temperatures
    std::vector<Real> initial_temps;
    for (int i = 0; i < _cellIndices.size(); ++i) {
      double T;
      openmc_cell_get_temperature(_cellIndices[i], &(_cellInstances[i]), &T);
      initial_temps.push_back(T);
    }
    auto & average_temp = getUserObject<NearestPointReceiver>("average_temp");
    average_temp.setValues(initial_temps);
}

void OpenMCProblem::externalSolve()
{
  if (openmc::mpi::master) {
    openmc::write_summary();
  }
  openmc_run();
}

void OpenMCProblem::syncSolutions(ExternalProblem::Direction direction)
{
  switch (direction)
  {
    case ExternalProblem::Direction::TO_EXTERNAL_APP:
    {
      auto & average_temp = getUserObject<NearestPointReceiver>("average_temp");
      // std::cout << "Temperatures: ";

      for (int i = 0; i < _cellIndices.size(); ++i)
      {
        auto& cell = openmc::model::cells[_cellIndices[i]];
        double T = average_temp.spatialValue(_centers[i]);
        std::cout << "Temperature at location: "
                  << _centers[i](0) << ' '
                  << _centers[i](1) << ' '
                  << _centers[i](2) <<
                  " Temp: " << T << std::endl;

        // std::cout << "Temperature: " << T << std::endl;
        // std::cout << "Cell instance: " << _cellInstances[i] << std::endl;
        cell->set_temperature(T, _cellInstances[i], true);
      }
      // std::cout << std::endl;

      for (auto& cell : openmc::model::cells) {
        if (cell->type_ == openmc::Fill::MATERIAL) {
          auto val = cell->sqrtkT_[0];
          // std::cout << val*val / openmc::K_BOLTZMANN << ' ';
        }
      }
      // std::cout << std::endl;
      break;
    }
    case ExternalProblem::Direction::FROM_EXTERNAL_APP:
    {

      auto& solution = _aux->solution();

      auto sys_number = _aux->number();

      // get the transfer mesh
      auto & transfer_mesh = _mesh.getMesh();

      if (_tallyType == TallyType::CELL) {
        auto cell_heat = cellHeatSource();
        // Debug
        // std::cout << "Cell heat source: " << std::endl;
        // for (auto val : cell_heat) { std::cout << val << " " << std::endl; }
        unsigned int elems_per_sphere = mesh().nElem() / _centers.size();

        for (unsigned int i = 0; i < _centers.size(); ++i ) {
          unsigned int offset = i * elems_per_sphere;
          for (unsigned int e = 0; e < elems_per_sphere; ++e) {
            auto elem_ptr = transfer_mesh.elem_ptr(offset + e);
            auto dof_idx = elem_ptr->dof_number(sys_number, _heat_source_var, 0);
            // set every element in this pebble with the same heating value
            solution.set(dof_idx, cell_heat.at(i));
          }

        }
      } else {
        // retrieve the heat source values
        auto mesh_heat = meshHeatSource();

        // Debug
        // std::cout << "Mesh heat source: " << std::endl;
        // for (auto val : mesh_heat) { std::cout << val << " " << std::endl; }


        // set the heat source directly on the mesh elements
        for (unsigned int i = 0; i < _meshFilters.size(); ++i) {
          auto& mesh_filter = _meshFilters[i];
          unsigned int offset = i * mesh_filter->n_bins();

          for (unsigned int e = 0; e < mesh_filter->n_bins(); ++e) {
            auto elem_ptr = transfer_mesh.elem_ptr(offset + e);
            auto dof_idx = elem_ptr->dof_number(sys_number, _heat_source_var, 0);
            solution.set(dof_idx, mesh_heat.at(e));
          }
        }
      }

      solution.close();

      break;
    }
    default:
    {
      mooseError("Shouldn't get here!");
      break;
    }
  }
}

std::vector<double> OpenMCProblem::meshHeatSource() {
  // determine the size of the heat source
  size_t heat_source_size = 0;
  for (const auto& t : _tallies) { heat_source_size += t->n_filter_bins(); }
  xt::xarray<double> heat = xt::zeros<double> ({heat_source_size});

  size_t idx = 0;
  double totalHeat = 0.0;
  for (int i = 0; i < _tallies.size(); i++) {
    const auto& tally = _tallies[i];
    // Determine number of realizations for normalizing tallies
    auto tally_mean = xt::view(tally->results_, xt::all(), 0, static_cast<int>(openmc::TallyResult::SUM));
    double tally_sum = std::accumulate(tally_mean.begin(), tally_mean.end(), 0.0);

    int m = tally->n_realizations_;
    // TODO: Change OpenMC so that it's correct on all ranks
    MPI_Bcast(&m, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // normalize by volume
    for (int bin = 0; bin < tally->n_filter_bins(); bin++) {
      heat(idx) = tally_mean(bin) * JOULE_PER_EV / m;
      totalHeat += heat(idx);
      idx++;
    }
  }

  double total_heat = xt::sum(heat)();
  double f = _power / total_heat;
  if (totalHeat != 0.0) {
    // normalize heat generation using power level
    int idx = 0;
    for (const auto& tally : _tallies) {
      for (int bin = 0; bin < tally->n_filter_bins(); bin++){
        heat(idx) *=  f / _meshTemplate->volume(bin);
        idx++;
      }
    }
  } else {
    heat = xt::zeros_like(heat);
  }

  // std::cout << "Heat source: ";
  // for (auto val : heat) { std::cout << val << ' '; }
  // std::cout << std::endl;

  return std::vector<double>(heat.begin(), heat.end());
}

std::vector<double> OpenMCProblem::cellHeatSource()
{
  auto tally = _tallies.at(0);

  // Determine number of realizations for normalizing tallies
  int m = tally->n_realizations_;
  // Broadcast number of realizations
  // TODO: Change OpenMC so that it's correct on all ranks
  MPI_Bcast(&m, 1, MPI_INT, 0, MPI_COMM_WORLD);

  // Determine energy production in each material
  auto meanValue = xt::view(tally->results_, xt::all(), 0, static_cast<int>(openmc::TallyResult::SUM));

  xt::xarray<double> heat = meanValue;
  heat *= JOULE_PER_EV;
  heat /= m;

  // Get total heat production [J/source]
  double totalHeat = xt::sum(heat)();

  if (totalHeat != 0.0) {
    // Normalize heat source in each material and collect in an array
    for (int i = 0; i < _cellIndices.size(); ++i) {
      double V = _volumes[i];
      // Convert heat from [J/source] to [W/cm^3]. Dividing by total_heat gives
      // the fraction of heat deposited in each material. Multiplying by power
      // givens an absolute value in W
      heat(i) *= _power / (totalHeat * V);
    }
  } else {
    heat = xt::zeros_like(heat);
  }

  std::cout << "Heat source: ";
  for (auto val : heat) { std::cout << val << ' '; }
  std::cout << std::endl;

  return std::vector<double>(heat.begin(), heat.end());
}

double OpenMCProblem::getCellVolume(int cellIndex) {
  int fillType = static_cast<int>(openmc::Fill::MATERIAL);
  int32_t *matIndices = nullptr;
  int nMat = 0;
  openmc_cell_get_fill(cellIndex, &fillType, &matIndices, &nMat);
  double vol = 0;
  for (int i = 0; i < nMat; ++i) {
    double thisVol = 0;
    openmc_material_get_volume(matIndices[i], &thisVol);
    vol += thisVol;
  }
  return vol;
}
