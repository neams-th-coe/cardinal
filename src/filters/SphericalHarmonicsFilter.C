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

#include "SphericalHarmonicsFilter.h"

#include "openmc/tallies/filter_sph_harm.h"

registerMooseObject("CardinalApp", SphericalHarmonicsFilter);

InputParameters
SphericalHarmonicsFilter::validParams()
{
  auto params = FilterBase::validParams();
  params.addClassDescription(
      "A class which provides a thin wrapper around an OpenMC SphericalHarmonicsFilter.");
  params.addRequiredRangeCheckedParam<unsigned int>(
      "order",
      "order <= 10",
      "The order of the spherical harmonics expansion. OpenMC only supports up to order 10.");

  return params;
}

SphericalHarmonicsFilter::SphericalHarmonicsFilter(const InputParameters & parameters)
  : FilterBase(parameters), _order(getParam<unsigned int>("order"))
{
  auto sh_filter = dynamic_cast<openmc::SphericalHarmonicsFilter *>(
      openmc::Filter::create("sphericalharmonics"));
  sh_filter->set_order(_order);
  sh_filter->set_cosine("particle");
  _filter = sh_filter;
}

std::string
SphericalHarmonicsFilter::binName(unsigned int bin_index) const
{
  unsigned int num_mom = 0;
  for (unsigned int l = 0; l <= _order; ++l)
  {
    for (int m = -1 * static_cast<int>(l); m < 0; ++m)
    {
      if (num_mom == bin_index)
        return "l" + Moose::stringify(l) + "_mneg" + Moose::stringify(-1 * m);
      num_mom++;
    }
    for (unsigned int m = 0; m <= l; ++m)
    {
      if (num_mom == bin_index)
        return "l" + Moose::stringify(l) + "_mpos" + Moose::stringify(m);
      num_mom++;
    }
  }

  return "";
}
#endif
