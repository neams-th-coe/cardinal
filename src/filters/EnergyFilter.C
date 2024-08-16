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
  params.addClassDescription("A class which provides a thin wrapper around an OpenMC EnergyFilter.");
  params.addRequiredParam<std::vector<Real>>("energy_boundaries",
    "The energy boundaries to use to form energy bins. The boundaries must be provided "
    "in ascending order in terms of energy.");
  params.addParam<bool>(
    "reverse_bins",
    false,
    "Whether the bins should be output in reverse order. This is useful for comparing Cardinal results "
    "with a deterministic transport code, where the convention is that energy groups are arranged in "
    "descending order (i.e. group 1 is fast, group 2 is thermal).");

  return params;
}

EnergyFilter::EnergyFilter(const InputParameters & parameters)
  : FilterBase(parameters),
    _energy_bnds(getParam<std::vector<Real>>("energy_boundaries")),
    _reverse_bins(getParam<bool>("reverse_bins"))
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

std::string
EnergyFilter::binName(unsigned int bin_index) const
{
  return "g" + _reverse_bins ? Moose::stringify(_energy_bnds.size() - bin_index - 1) : Moose::stringify(bin_index + 1);
}
#endif
