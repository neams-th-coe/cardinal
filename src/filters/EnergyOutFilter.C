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
#include "EnergyOutFilter.h"

#include "EnergyGroupStructures.h"
#include "EnergyFilter.h"
#include "CardinalEnums.h"

#include "openmc/tallies/filter_energy.h"

registerMooseObject("CardinalApp", EnergyOutFilter);

InputParameters
EnergyOutFilter::validParams()
{
  auto params = FilterBase::validParams();
  params.addClassDescription(
      "A class which provides a thin wrapper around an OpenMC EnergyOutFilter. Energy bins "
      "can either be manually specified in 'energy_boundaries' or picked from a list "
      "provided in 'group_structure'.");
  params.addParam<std::vector<Real>>(
      "energy_boundaries",
      "The energy boundaries to use to form energy bins. The boundaries must be provided "
      "in ascending order in terms of energy.");
  params.addParam<MooseEnum>(
      "group_structure",
      getEnergyGroupsEnum(),
      "The energy group structure to use from a list of popular group structures.");
  params.addParam<bool>(
      "reverse_bins",
      false,
      "Whether the bins should be output in reverse order. This is useful for comparing "
      "deterministic transport codes with Cardinal, where the convention is that energy "
      "groups are arranged in descending order (i.e. group 1 is fast, group 2 is thermal).");

  return params;
}

EnergyOutFilter::EnergyOutFilter(const InputParameters & parameters)
  : FilterBase(parameters), _reverse_bins(getParam<bool>("reverse_bins"))
{
  if (isParamValid("energy_boundaries") == isParamValid("group_structure"))
    mooseError("You have either set both 'energy_boundaries' and 'group_structure' or have not "
               "specified a bin option. Please specify either 'energy_boundaries' or "
               "'group_structure'.");

  if (isParamValid("energy_boundaries"))
    _energy_bnds = getParam<std::vector<Real>>("energy_boundaries");
  if (isParamValid("group_structure"))
    _energy_bnds = EnergyFilter::getGroupBoundaries(
        getParam<MooseEnum>("group_structure").getEnum<energyfilter::GroupStructureEnum>(), this);

  // Two boundaries are required at minimum to form energy bins.
  if (_energy_bnds.size() < 2)
    paramError("energy_boundaries",
               "At least two energy values are required to create energy bins!");

  // Check to make sure none of the boundaries are negative.
  for (const auto & bnd : _energy_bnds)
    if (bnd < 0.0)
      paramError("energy_boundaries",
                 "Energy group boundaries must be positive to create energy bins!");

  // Sort the boundaries so they're monotonically decreasing.
  std::sort(_energy_bnds.begin(),
            _energy_bnds.end(),
            [](const Real & a, const Real & b) { return a < b; });

  // Check for duplicate entries.
  if (std::adjacent_find(_energy_bnds.begin(), _energy_bnds.end()) != _energy_bnds.end())
    paramError("energy_boundaries",
               "You have added duplicate energy boundaries! Each group boundary must be unique to create energy bins.");

  // Initialize the OpenMC EnergyoutFilter.
  _filter_index = openmc::model::tally_filters.size();

  auto energy_out_filter =
      dynamic_cast<openmc::EnergyoutFilter *>(openmc::Filter::create("energyout"));
  energy_out_filter->set_bins(_energy_bnds);
  _filter = energy_out_filter;
}

std::string
EnergyOutFilter::binName(unsigned int bin_index) const
{
  return "gp" + (_reverse_bins ? Moose::stringify(_energy_bnds.size() - bin_index - 1)
                               : Moose::stringify(bin_index + 1));
}

#endif
