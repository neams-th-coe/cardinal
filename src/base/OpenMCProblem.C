//
// Created by Ronald Rahaman on 2019-01-23.
//

#include "OpenMCProblem.h"
#include "openmc/capi.h"
#include "openmc/particle.h"
#include "openmc/geometry.h"

registerMooseObject("OpenMCApp", OpenMCProblem);

template<>
InputParameters
validParams<OpenMCProblem>()
{
  InputParameters params = validParams<ExternalProblem>();
  params.addRequiredParam<Real>("power", "specified power for OpenMC");
  params.addRequiredParam<std::vector<Point>>("centers", "coords of pebble centers");
  return params;
}

OpenMCProblem::OpenMCProblem(const InputParameters &params) : 
  ExternalProblem(params), 
  _centers(getParam<std::vector<Point>>("centers")), 
  _filter_id(get_filter_id()),
  _index_filter(get_index_filter(_filter_id)),
  _tally_id(get_tally_id()),
  _index_tally(get_index_tally(_tally_id)),
  _filter(dynamic_cast<openmc::CellFilter*>(openmc::model::tally_filters[_index_filter].get())),
  _tally(openmc::model::tallies[_index_tally])
{
  // Find cell for each pebble center
  // _centers is initialized with the pebble centers from .i file
  for (auto &c : _centers) {
    openmc::Particle p {};
    p.r() = {c(0), c(1), c(2)};
    p.u() = {0., 0., 1.};
    openmc::find_cell(&p, false);
    _cell_indices.push_back(p.coord_[0].cell);
    _cell_instances.push_back(p.cell_instance_);
  }

  // Create cell filter
  _filter->cells_ = _cell_indices;
  _filter->n_bins_ = _filter->cells_.size();
  for (int i = 0; i < _filter->cells_.size(); ++i) {
    _filter->map_[_filter->cells_[i]] = i;
  }

  // Create fission tally
  _tally->set_filters(&_index_filter, 1);
  _tally->set_scores({"kappa-fission"});
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
      for (int i=0; i < _cell_indices.size(); ++i) {
        // TODO:  Get temperature from BISON
        double T = 295.0;
        openmc_cell_set_temperature(_cell_indices[i], T, &(_cell_instances[i]));
      }
      break;
    }
    case ExternalProblem::Direction::FROM_EXTERNAL_APP:
    {
      break;
    }
    default:
    {
      mooseError("Shouldn't get here!");
      break;
    }
  }
}

int32_t get_filter_id()
{
  int32_t filter_id;
  openmc_get_filter_next_id(&filter_id);
  return filter_id;
}

int32_t get_index_filter(int32_t filter_id)
{
  int32_t index_filter;
  openmc_new_filter("cell", &index_filter);
  openmc_filter_set_id(index_filter, filter_id);
  return index_filter;
}

int32_t get_tally_id()
{
  int32_t tally_id;
  openmc_get_tally_next_id(&tally_id);
  return tally_id;
}

int32_t get_index_tally(int32_t tally_id) 
{
  int32_t index_tally;
  openmc_extend_tallies(1, &index_tally, nullptr);
  openmc_tally_set_id(index_tally, tally_id);
  return index_tally;
}
