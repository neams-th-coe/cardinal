//
// Created by Ronald Rahaman on 2019-01-23.
//

#include "NearestPointReceiver.h"

#include "mpi.h"
#include "OpenMCProblem.h"
#include "openmc/capi.h"
#include "openmc/cell.h"
#include "openmc/constants.h"
#include "openmc/particle.h"
#include "openmc/geometry.h"
#include "openmc/settings.h"
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
    std::cerr << "Warning: LibMesh communicator already set in OpenMC." << std::endl;
  }

  openmc::settings::libmesh_comm = &m.comm();

    // Find cell for each pebble center
    // _centers is initialized with the pebble centers from .i file
    for (auto &c : _centers) {
      openmc::Particle p {};
      p.r() = {c(0), c(1), c(2)};
      p.u() = {0., 0., 1.};
      openmc::find_cell(&p, false);
      _cellIndices.push_back(p.coord_[p.n_coord_ - 1].cell);
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
  //mesh->output_ = false;

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
    _meshFilters.push_back(meshFilter);
    meshFilter->set_mesh(mesh_index);
    meshFilter->set_translation({c(0), c(1), c(2)});
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
}

void OpenMCProblem::addExternalVariables()
{

  // cell-based heat source
  if (_tallyType == TallyType::CELL)
  {
    std::cout << "Setting cell params" << std::endl;
    auto receiver_params = _factory.getValidParams("NearestPointReceiver");
    receiver_params.set<std::vector<Point>>("positions") = _centers;

    // Will be filled with values from OpenMC
    addUserObject("NearestPointReceiver", "heat_source", receiver_params);
  }
  // mesh-based heat source
  else if (_tallyType == TallyType::MESH)
  {
    // set points based on element centroids
    std::vector<Point> element_centers;

    for (const auto& mesh_filter : _meshFilters) {
      auto translation = mesh_filter->translation();
      const auto& mesh = openmc::model::meshes[mesh_filter->mesh()];
      const auto umesh = dynamic_cast<openmc::LibMesh*>(mesh.get());

      for (int bin = 0; bin < mesh_filter->n_bins(); bin++) {
        auto centroid = umesh->centroid(bin);
        if (mesh_filter->translated()) { centroid += translation; }
        element_centers.push_back({centroid[0], centroid[1], centroid[2]});
      }
    }
    auto receiver_params = _factory.getValidParams("NearestPointReceiver");
    receiver_params.set<std::vector<Point>>("positions") = element_centers;
    // Will be filled with values from OpenMC
    addUserObject("NearestPointReceiver", "heat_source", receiver_params);
  }

  {
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
}

void OpenMCProblem::externalSolve()
{
  openmc_run();
}

void OpenMCProblem::syncSolutions(ExternalProblem::Direction direction)
{
  switch (direction)
  {
    case ExternalProblem::Direction::TO_EXTERNAL_APP:
    {
      auto & average_temp = getUserObject<NearestPointReceiver>("average_temp");
      for (int i=0; i < _cellIndices.size(); ++i)
      {
        double T = average_temp.spatialValue(_centers[i]);
        openmc_cell_set_temperature(_cellIndices[i], T, &(_cellInstances[i]));
      }
      break;
    }
    case ExternalProblem::Direction::FROM_EXTERNAL_APP:
    {
      auto& receiver = getUserObject<NearestPointReceiver>("heat_source");
      if (_tallyType == TallyType::CELL) {
        auto heat = heat_source();
        std::cout << "Cell heat source: " << std::endl;
        for (auto& val : heat) { std::cout << val << " "; }
        std::cout << std::endl;
        receiver.setValues(heat);
      } else {
        auto mesh_heat = mesh_heat_source();
        std::cout << "Mesh heat source: " << std::endl;
        for (auto& val : mesh_heat) { std::cout << val << " "; }
        std::cout << std::endl;
        receiver.setValues(mesh_heat);
      }
      break;
    }
    default:
    {
      mooseError("Shouldn't get here!");
      break;
    }
  }
}

std::vector<double> OpenMCProblem::mesh_heat_source() {
  std::cout << "Tallies size: " << _tallies.size();
  // determine the size of the heat source
  size_t heat_source_size = 0;
  for (const auto& t : _tallies) { heat_source_size += t->n_filter_bins(); }
  xt::xarray<double> heat = xt::zeros<double> ({heat_source_size});
  std::cout << "Heat source size: " << heat_source_size << std::endl;

  size_t idx = 0;
  for (int i = 0; i < _tallies.size(); i++) {
    const auto& tally = _tallies[i];
    // Determine number of realizations for normalizing tallies
    auto tally_mean = xt::view(tally->results_, xt::all(), 0, static_cast<int>(openmc::TallyResult::SUM));

    int m = tally->n_realizations_;
    // normalize by volume
    for (int bin = 0; bin < tally->n_filter_bins(); bin++) {
      heat(idx++) = tally_mean(bin) / (m * _meshTemplate->volume(bin));
    }
  }

  const double JOULE_PER_EV = 1.6021766208e-19;
  double totalHeat = xt::sum(heat)();

  if (totalHeat != 0.0) {
    // normalize heat generation using power level
    heat *=  JOULE_PER_EV * _power / totalHeat;
  } else {
    heat = xt::zeros_like(heat);
  }

  return std::vector<double>(heat.begin(), heat.end());
}

std::vector<double> OpenMCProblem::heat_source()
{
  auto tally = _tallies.at(0);

  // Determine number of realizations for normalizing tallies
  int m = tally->n_realizations_;
  std::cout << "Realizations: " << m << std::endl;
  // Broadcast number of realizations
  // TODO: Change OpenMC so that it's correct on all ranks
  MPI_Bcast(&m, 1, MPI_INT, 0, MPI_COMM_WORLD);

  // Determine energy production in each material
  auto meanValue = xt::view(tally->results_, xt::all(), 0, static_cast<int>(openmc::TallyResult::SUM));
  std::cout << "Mean: " << std::endl;
  for (auto& val : meanValue) { std::cout << val << " "; }
  std::cout << std::endl;

  const double JOULE_PER_EV = 1.6021766208e-19;
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

  return std::vector<double>(heat.begin(), heat.end());
}

double OpenMCProblem::get_cell_volume(int cellIndex) {
  int fillType {};
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
