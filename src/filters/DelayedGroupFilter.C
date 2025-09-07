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
#include "DelayedGroupFilter.h"

#include "OpenMCCellAverageProblem.h"

#include "openmc/tallies/filter_delayedgroup.h"

registerMooseObject("CardinalApp", DelayedGroupFilter);

InputParameters
DelayedGroupFilter::validParams()
{
  auto params = FilterBase::validParams();
  params.addClassDescription(
      "A class which provides a thin wrapper around an OpenMC DelayedGroupFilter.");
  params.addRequiredRangeCheckedParam<std::vector<int>>(
      "dnp_groups",
      "dnp_groups > 0 & dnp_groups < 7",
      "The delayed neutron precursor bins to filter for. Valid bin indices are integers between 1 "
      "and 6 (inclusive).");

  return params;
}

DelayedGroupFilter::DelayedGroupFilter(const InputParameters & parameters)
  : FilterBase(parameters),
    _delayed_groups(getParam<std::vector<int>>("dnp_groups"))
{
  _openmc_problem.checkEmptyVector(_delayed_groups, "dnp_groups");

  // Initialize the OpenMC DelayedGroupFilter.
  _filter_index = openmc::model::tally_filters.size();

  auto dnp_grp_filter = dynamic_cast<openmc::DelayedGroupFilter *>(openmc::Filter::create("delayedgroup"));
  try
  {
    dnp_grp_filter->set_groups(openmc::span<int>(&_delayed_groups[0], _delayed_groups.size()));
  }
  catch (const std::exception & e)
  {
    paramError("dnp_groups", e.what());
  }
  _filter = dnp_grp_filter;
}

std::string
DelayedGroupFilter::binName(unsigned int bin_index) const
{
  return "d" + Moose::stringify(bin_index + 1);
}
#endif
