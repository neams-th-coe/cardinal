//
// Created by Ronald Rahaman on 2019-01-23.
//

#include "NearestPointReceiver.h"

#include "mpi.h"
#include "OpenMCProblem.h"
#include "openmc/capi.h"
#include "openmc/cell.h"
#include "openmc/particle.h"
#include "openmc/geometry.h"
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
  return params;
}

OpenMCProblem::OpenMCProblem(const InputParameters &params) :
  ExternalProblem(params),
  _centers(getParam<std::vector<Point>>("centers")),
  _power(getParam<Real>("power")),
  _volumes(getParam<std::vector<Real>>("volumes")),
  _filterId(getFilterId()),
  _filterIndex(getNewFilter(_filterId, "cell")),
  _tallyId(getTallyId()),
  _tallyIndex(getNewTally(_tallyId)),
  _filter(dynamic_cast<openmc::CellFilter*>(openmc::model::tally_filters[_filterIndex].get())),
  _tally(openmc::model::tallies[_tallyIndex])
{
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
  _tally->set_filters(filter_indices);
  _tally->set_scores({"kappa-fission"});
}


void OpenMCProblem::addExternalVariables()
{
  {
    auto receiver_params = _factory.getValidParams("NearestPointReceiver");
    receiver_params.set<std::vector<Point>>("positions") = _centers;

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
        std::cout << "Temp: " << T << std::endl;
        openmc_cell_set_temperature(_cellIndices[i], T, &(_cellInstances[i]));
      }
      break;
    }
    case ExternalProblem::Direction::FROM_EXTERNAL_APP:
    {
      // This is an xtensor of heat values per cell
      auto heat = heat_source();

      std::vector<Real> values;
      values.reserve(heat.size());

      for (auto & val : heat)
        values.push_back(val);

      auto & receiver = getUserObject<NearestPointReceiver>("heat_source");

      receiver.setValues(values);

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

xt::xtensor<double, 1> OpenMCProblem::heat_source()
{
  // Determine number of realizatoins for normalizing tallies
  int m = _tally->n_realizations_;

  // Broadcast number of realizations
  // TODO: Change OpenMC so that it's correct on all ranks
  MPI_Bcast(&m, 1, MPI_INT, 0, MPI_COMM_WORLD);

  // Determine energy production in each material
  auto meanValue = xt::view(_tally->results_, xt::all(), 0, openmc::TallyResult::SUM);
  const double JOULE_PER_EV = 1.6021766208e-19;
  xt::xtensor<double, 1> heat = meanValue;
  heat *= JOULE_PER_EV;
  heat /= m;

  // Get total heat production [J/source]
  double totalHeat = xt::sum(heat)();

  // Debug
//  _console << "*** Power: " << _power << std::endl;
//  _console << "*** Total heat: " << totalHeat << std::endl;

  // Normalize heat source in each material and collect in an array
  for (int i = 0; i < _cellIndices.size(); ++i) {
    double V = _volumes[i];
    // Convert heat from [J/source] to [W/cm^3]. Dividing by total_heat gives
    // the fraction of heat deposited in each material. Multiplying by power
    // givens an absolute value in W
    heat(i) *= _power / (totalHeat * V);

    // Debug
//    _console << "*** Volume[" << i << "]: " << V << std::endl;
//    _console << "*** Heat[" << i << "]: " << heat(i) << std::endl;
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
