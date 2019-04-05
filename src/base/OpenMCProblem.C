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
  _filter_index(get_new_filter(_filter_id, "cell")),
  _tally_id(get_tally_id()),
  _tally_index(get_new_tally(_tally_id)),
  _filter(dynamic_cast<openmc::CellFilter*>(openmc::model::tally_filters[_filter_index].get())),
  _tally(openmc::model::tallies[_tally_index])
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

  // Setup cell filter
  _filter->cells_ = _cell_indices;
  _filter->n_bins_ = _filter->cells_.size();
  for (int i = 0; i < _filter->cells_.size(); ++i) {
    _filter->map_[_filter->cells_[i]] = i;
  }

  // Setup fission tally
  _tally->set_filters(&_filter_index, 1);
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


// xt::xtensor<double, 3> OpenMCProblem::tally_results()
// {
//   // Get material bins
//   int32_t* mats;
//   int32_t n_mats;
//   openmc_material_filter_get_bins(_filter, &mats, &n_mats);
// 
//   // Get tally results and number of realizations
//   double* results;
//   std::array<std::size_t, 3> shape;
//   openmc_tally_results(index_tally_, &results, shape.data());
//   int32_t m;
//   err_chk(openmc_tally_get_n_realizations(index_tally_, &m));
// 
//   // Determine size
//   std::size_t size {shape[0] * shape[1] * shape[2]};
// 
//   // Adapt array into xtensor with no ownership
//   return xt::adapt(results, size, xt::no_ownership(), shape);
// }


//! Queries the next available filter ID from OpenMC
//! \return The next available filter ID
int32_t get_filter_id()
{
  int32_t filter_id;
  openmc_get_filter_next_id(&filter_id);
  return filter_id;
}

//! Allocates a new filter with specified type in OpenMC
//! \param[in] The ID of the newly-constructed cell filter
//! \param[in] The type of the newly-constructed cell filter
//! \return The index of the new filter in OpenMC's filter array
int32_t get_new_filter(int32_t filter_id, const char *type)
{
  int32_t filter_index;
  openmc_new_filter(type, &filter_index);
  openmc_filter_set_id(filter_index, filter_id);
  return filter_index;
}

//! Queries the next available tally ID from OpenMC
//! \return The next available tally ID
int32_t get_tally_id()
{
  int32_t tally_id;
  openmc_get_tally_next_id(&tally_id);
  return tally_id;
}

//! Allocates a new tally with unspecified scores/filters in OpenMC
//! \param[in] The ID of the newly-constructed tally
//! \return The index of the new tally in OpenMC's tally array
int32_t get_new_tally(int32_t tally_id) 
{
  int32_t index_tally;
  openmc_extend_tallies(1, &index_tally, nullptr);
  openmc_tally_set_id(index_tally, tally_id);
  return index_tally;
}
