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

#include "FromXMLFilter.h"

registerMooseObject("CardinalApp", FromXMLFilter);

InputParameters
FromXMLFilter::validParams()
{
  auto params = FilterBase::validParams();
  params.addClassDescription("A class which provides a thin wrapper around an arbitrary OpenMC filter.");
  params.addRequiredRangeCheckedParam<int>(
    "filter_id",
    "filter_id >= 0",
    "The id of the OpenMC filter that this class should provide to Cardinal tallies.");
  params.addRequiredParam<std::string>("bin_label", "The label that is used for this filter's bins.");
  params.addParam<bool>(
    "allow_expansion_filters",
    false,
    "Whether functional expansion filters are allowed or not. Tallies with these filters are likely to fail "
    "normalization as a sum over all bins may not make sense for a certain functional expansion.");

  return params;
}

FromXMLFilter::FromXMLFilter(const InputParameters & parameters)
  : FilterBase(parameters),
    _filter_id(getParam<int>("filter_id")),
    _bin_label(getParam<std::string>("bin_label"))
{
  // Check to make sure the filter exists.
  if (openmc::model::filter_map.count(_filter_id) == 0)
    mooseError("A filter with the id "
               + Moose::stringify(_filter_id)
               + " does not exist in the OpenMC model! Please make sure the filter has been "
               "added in the OpenMC model and you've supplied the correct filter id.");

  _filter_index = openmc::model::filter_map.at(_filter_id);
  _filter = openmc::model::tally_filters[_filter_index].get();

  // Check to see if the filter is an expansion filter.
  bool is_exp;
  switch (_filter->type())
  {
    case openmc::FilterType::LEGENDRE:            is_exp = true;  break;
    case openmc::FilterType::SPATIAL_LEGENDRE:    is_exp = true;  break;
    case openmc::FilterType::SPHERICAL_HARMONICS: is_exp = true;  break;
    case openmc::FilterType::ZERNIKE:             is_exp = true;  break;
    case openmc::FilterType::ZERNIKE_RADIAL:      is_exp = true;  break;
    case openmc::FilterType::ENERGY_FUNCTION:     is_exp = true;  break;
    default:                                      is_exp = false; break;
  }

  if (is_exp && getParam<bool>("allow_expansion_filters"))
    mooseWarning("You have selected a functional expansion filter. Tallies which use this filter "
                 "may fail normalization as the sum over all tally bins may not be well posed "
                 "if any bins contain functional expansion coefficients.");
  else if (is_exp && !getParam<bool>("allow_expansion_filters"))
    mooseError("You have selected a functional expansion filter. Tallies which use this filter "
               "may fail normalization as the sum over all tally bins may not be well posed "
               "if any bins contain functional expansion coefficients. If you still wish to "
               "use this filter, set 'allow_expansion_filters' to true.");
}
#endif
