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

#include "AngularLegendreFilter.h"

#include "openmc/tallies/filter_legendre.h"

registerMooseObject("CardinalApp", AngularLegendreFilter);

InputParameters
AngularLegendreFilter::validParams()
{
  auto params = FilterBase::validParams();
  params.addClassDescription(
      "A class which provides a thin wrapper around an OpenMC LegendreFilter.");
  params.addRequiredParam<unsigned int>("order", "The order of the Legendre expansion.");

  return params;
}

AngularLegendreFilter::AngularLegendreFilter(const InputParameters & parameters)
  : FilterBase(parameters), _order(getParam<unsigned int>("order"))
{
  auto legendre_filter = dynamic_cast<openmc::LegendreFilter *>(openmc::Filter::create("legendre"));

  legendre_filter->set_order(_order);
  _filter = legendre_filter;
}

std::string
AngularLegendreFilter::binName(unsigned int bin_index) const
{
  return "l" + Moose::stringify(bin_index);
}

#endif
