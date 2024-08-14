/********************************************************************/
/*                  SOFTWARE COPYRIGHT NOTIFICATION                 */
/*                             Cardinal                             */
/*                                                                  */
/*                  (c) 2021 UChicago Argonne, LLC                  */
/*                        ALL RIGHTS RESERVED                       */
/*                                                                  */
/*                 Prepared by UChicago Argonne, LLC                */
/*               Under Contract No. DE-AC02-06CH11357               */
/*                With the U. S. Department of Energy               */
/*                                                                  */
/*             Prepared by Battelle Energy Alliance, LLC            */
/*               Under Contract No. DE-AC07-05ID14517               */
/*                With the U. S. Department of Energy               */
/*                                                                  */
/*                 See LICENSE for full restrictions                */
/********************************************************************/

#ifdef ENABLE_OPENMC_COUPLING
#include "EnergyFilter.h"

#include "openmc/tallies/filter_energy.h"

registerMooseObject("CardinalApp", EnergyFilter);

InputParameters
EnergyFilter::validParams()
{
  auto params = FilterBase::validParams();
  params.addClassDescription("A class which implements an OpenMC EnergyFilter.");
  params.addRequiredParam<std::vector<Real>>("energy_boundaries",
    "The energy boundaries to use to form energy bins. The boundaries must be provided "
    "in ascending order in terms of energy.");

  return params;
}

EnergyFilter::EnergyFilter(const InputParameters & parameters)
  : FilterBase(parameters),
    _energy_bnds(getParam<std::vector<Real>>("energy_boundaries"))
{
  // Two boundaries are required at minimum to form energy bins.
  if (_energy_bnds.size() < 2)
    paramError("energy_boundaries", "At least two energy values are required to create energy bins!");

  // Sanity check the energy boundaries to make sure they're increasing monotonically.
  for (unsigned int i = 0u; i < _energy_bnds.size() - 1; ++i)
    if (_energy_bnds[i] >= _energy_bnds[i + 1])
      paramError("energy_boundaries", "The energy boundaries must be provided in ascending order in terms of energy!");

  // Initialize the OpenMC EnergyFilter.
  _filter_index = openmc::model::tally_filters.size();

  auto energy_filter = dynamic_cast<openmc::EnergyFilter *>(openmc::Filter::create("energy"));
  energy_filter->set_bins(_energy_bnds);
  _filter = energy_filter;
}
#endif
