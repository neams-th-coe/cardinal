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
  params.addRequiredParam<std::string>("mesh_template", "mesh tally template for OpenMC");
  return params;
}

OpenMCProblem::OpenMCProblem(const InputParameters &params) :
  ExternalProblem(params),
  _centers(getParam<std::vector<Point>>("centers")),
  _power(getParam<Real>("power")),
  _volumes(getParam<std::vector<Real>>("volumes")),
  _meshTemplateFilename(getParam<std::string>("mesh_template")),
  _filterId(getFilterId()),
  _filterIndex(getNewFilter(_filterId, "cell")),
  _tallyId(getTallyId()),
  _tallyIndex(getNewTally(_tallyId)),
  _filter(dynamic_cast<openmc::CellFilter*>(openmc::model::tally_filters[_filterIndex].get()))
{

  int err;


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

  // Setup cell filter
  _filter->set_cells(_cellIndices);

  // Setup fission tally
  std::vector<openmc::Filter*> filter_indices = {_filter};
  auto& tally = openmc::model::tallies[_tallyIndex];
  tally->set_filters(filter_indices);
  tally->set_scores({"kappa-fission"});

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
    auto meshFilter = std::make_unique<openmc::MeshFilter>();
    meshFilter->set_mesh(mesh_index);
    meshFilter->set_id(meshFilterId++);
    meshFilter->set_translation({c(0), c(1), c(2)});
    openmc::model::filter_map[meshFilter->id()] = openmc::model::tally_filters.size();
    _meshFilters.push_back(meshFilter.get());
    std::vector<openmc::Filter*> filters = {meshFilter.get()};
    openmc::model::tally_filters.push_back(std::move(meshFilter));

    // apply the mesh filter to a tally
    auto tally = openmc::Tally::create(meshTallyId++);
    tally->set_filters(filters);
    tally->set_scores({"kappa-fission"});
    tally->estimator_ = openmc::TallyEstimator::COLLISION;
    _meshTallies.push_back(tally);

    // add to openmc
  }
}

void OpenMCProblem::addExternalVariables()
{

  // cell-based heat source
  {
    auto receiver_params = _factory.getValidParams("NearestPointReceiver");
    receiver_params.set<std::vector<Point>>("positions") = _centers;

    // Will be filled with values from OpenMC
    addUserObject("NearestPointReceiver", "heat_source", receiver_params);
  }

  // mesh-based heat source
  {
    // set points based on element centroids
    std::vector<Point> element_centers;

    for (const auto& tally : _meshTallies) {
      // get the mesh tally filter
      const auto& filter = openmc::model::tally_filters[tally->filters(0)];
      const auto mesh_filter = dynamic_cast<openmc::MeshFilter*>(filter.get());
      auto translation = mesh_filter->translation();
      const auto& mesh = openmc::model::meshes[mesh_filter->mesh()];
      const auto umesh = dynamic_cast<openmc::LibMesh*>(mesh.get());

      for (int bin = 0; bin < mesh_filter->n_bins(); bin++) {
        auto center = umesh->centroid(bin);
        if (mesh_filter->translated()) { center += translation; }
        element_centers.push_back({center[0], center[1], center[2]});
      }
    }
    auto receiver_params = _factory.getValidParams("NearestPointReceiver");
    receiver_params.set<std::vector<Point>>("positions") = element_centers;

    // Will be filled with values from OpenMC
    addUserObject("NearestPointReceiver", "mesh_heat_source", receiver_params);
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
      auto mesh_heat = mesh_heat_source();
      auto & mesh_receiver = getUserObject<NearestPointReceiver>("mesh_heat_source");
      mesh_receiver.setValues(mesh_heat);
      break;
    }
    default:
    {
      mooseError("Shouldn't get here!");
      break;
    }
  }
}

//! Queries the next available filter ID from OpenMC
//! \return The next available filter ID
int32_t OpenMCProblem::getFilterId()
{
  int32_t filterId;
  openmc_get_filter_next_id(&filterId);
  return filterId;
}

//! Allocates a new filter with specified type in OpenMC
//! \param[in] The ID of the newly-constructed cell filter
//! \param[in] The type of the newly-constructed cell filter
//! \return The index of the new filter in OpenMC's filter array
int32_t OpenMCProblem::getNewFilter(int32_t filterId, const char *type)
{
  int32_t filterIndex;
  openmc_new_filter(type, &filterIndex);
  openmc_filter_set_id(filterIndex, filterId);
  return filterIndex;
}

//! Queries the next available tally ID from OpenMC
//! \return The next available tally ID
int32_t OpenMCProblem::getTallyId()
{
  int32_t tallyId;
  openmc_get_tally_next_id(&tallyId);
  return tallyId;
}

//! Allocates a new tally with unspecified scores/filters in OpenMC
//! \param[in] The ID of the newly-constructed tally
//! \return The index of the new tally in OpenMC's tally array
int32_t OpenMCProblem::getNewTally(int32_t tallyId)
{
  int32_t index_tally;
  openmc_extend_tallies(1, &index_tally, nullptr);
  openmc_tally_set_id(index_tally, tallyId);
  return index_tally;
}

std::vector<double> OpenMCProblem::mesh_heat_source() {
  // determine the size of the xtensor
  size_t heat_source_size = _meshTemplate->n_bins() * _meshTallies.size();
  xt::xarray<double> heat = xt::zeros<double> ({heat_source_size});

  for (int i = 0; i < _meshTallies.size(); i++) {
    const auto& tally = _meshTallies[i];
    // Determine number of realizations for normalizing tallies
    auto tally_mean = xt::view(tally->results_, xt::all(), 0, static_cast<int>(openmc::TallyResult::SUM));

    auto heat_view = xt::view(heat, xt::range(tally->n_filter_bins() * i, tally->n_filter_bins() * (i+1)));

    int m = tally->n_realizations_;
    // normalize by volume
    for (int bin = 0; bin < tally->n_filter_bins(); bin++) {
      heat(tally->n_filter_bins() * i + bin) = tally_mean(bin) / (m * _meshTemplate->volume(bin));
    }
  }

  const double JOULE_PER_EV = 1.6021766208e-19;
  double totalHeat = xt::sum(heat)();

  // normalize heat generation using power level
  heat *=  JOULE_PER_EV * _power / totalHeat;

  return std::vector<double>(heat.begin(), heat.end());
}

xt::xtensor<double, 1> OpenMCProblem::heat_source()
{
  // Determine number of realizations for normalizing tallies
  int m = openmc::model::tallies[_tallyIndex]->n_realizations_;

  // Broadcast number of realizations
  // TODO: Change OpenMC so that it's correct on all ranks
  MPI_Bcast(&m, 1, MPI_INT, 0, MPI_COMM_WORLD);

  // Determine energy production in each material
  auto meanValue = xt::view(openmc::model::tallies[_tallyIndex]->results_, xt::all(), 0, static_cast<int>(openmc::TallyResult::SUM));
  const double JOULE_PER_EV = 1.6021766208e-19;
  xt::xtensor<double, 1> heat = meanValue;
  heat *= JOULE_PER_EV;
  heat /= m;

  // Get total heat production [J/source]
  double totalHeat = xt::sum(heat)();

  // Normalize heat source in each material and collect in an array
  for (int i = 0; i < _cellIndices.size(); ++i) {
    double V = _volumes[i];
    // Convert heat from [J/source] to [W/cm^3]. Dividing by total_heat gives
    // the fraction of heat deposited in each material. Multiplying by power
    // givens an absolute value in W
    heat(i) *= _power / (totalHeat * V);

  }

  return heat;
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
