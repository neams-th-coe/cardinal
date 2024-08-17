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

#include "AzimuthalAngleFilter.h"

#include "openmc/tallies/filter_azimuthal.h"

registerMooseObject("CardinalApp", AzimuthalAngleFilter);

InputParameters
AzimuthalAngleFilter::validParams()
{
  auto params = FilterBase::validParams();
  params.addClassDescription("A class which provides a thin wrapper around an OpenMC AzimuthalFilter. "
    "Bins can either be equally spaced by setting 'num_equal_divisions', or a set of azimuthal angles can "
    "be provided by setting 'azimuthal_angle_boundaries'.");
  params.addRangeCheckedParam<unsigned int>(
    "num_equal_divisions", "num_equal_divisions > 0", "The number of equally spaces subdivisions of "
    "$[-\\pi, \\pi)$ to use if equal subdivisions are desired.");
  params.addParam<std::vector<Real>>("azimuthal_angle_boundaries",
    "The azimuthal angle boundaries in $[-\\pi, \\pi)$ which must be provided in increasing order. If $-\\pi$ and "
    "$\\pi$ are not included this filter may result in some missed particles during normalization.");

  return params;
}

AzimuthalAngleFilter::AzimuthalAngleFilter(const InputParameters & parameters)
  : FilterBase(parameters)
{
  if (isParamValid("num_equal_divisions") && isParamValid("azimuthal_angle_boundaries"))
    mooseError("You have set both 'num_equal_divisions' and 'azimuthal_angle_boundaries'! Only one bin "
               "option can be specified at a time.");

  if (isParamValid("num_equal_divisions"))
  {
    auto num_angles = getParam<unsigned int>("num_equal_divisions");
    for (unsigned int i = 0; i < num_angles + 1; ++i)
      _azimuthal_angle_bnds.push_back((-libMesh::pi) + i * (2.0 * libMesh::pi / num_angles));
  }
  else if (isParamValid("azimuthal_angle_boundaries"))
  {
    const auto & bnds = getParam<std::vector<Real>>("azimuthal_angle_boundaries");

    // Make sure we have at least two boundaries to form bins.
    if (bnds.size() < 2)
      paramError("azimuthal_angle_boundaries", "At least two azimuthal angles are required to create bins!");

    // Sanity check the boundaries to make sure they're provided in increasing order.
    for (unsigned int i = 0; i < bnds.size() - 1; ++i)
    {
      if (bnds[i] > bnds[i + 1])
        paramError("azimuthal_angle_boundaries", "The azimuthal angle boundaries must be provided in increasing order!");

      _azimuthal_angle_bnds.push_back(bnds[i]);
    }
    _azimuthal_angle_bnds.push_back(bnds.back());

    // Warn the user if there is the possibility of missed particles.
    if (_azimuthal_angle_bnds.front() > (-1.0 * libMesh::pi) || _azimuthal_angle_bnds.back() < libMesh::pi)
      mooseWarning("The bin boundaries don't contain one of the following: -pi or pi. Some particles may be "
                   "missed during normalization.");
  }
  else
    mooseError("No bins have been specified! Please set either 'num_equal_divisions' or "
               "'azimuthal_angle_boundaries'.");

  // Initialize the OpenMC AzimuthalFilter.
  _filter_index = openmc::model::tally_filters.size();

  auto azimuthal_filter = dynamic_cast<openmc::AzimuthalFilter *>(openmc::Filter::create("azimuthal"));
  azimuthal_filter->set_bins(_azimuthal_angle_bnds);
  _filter = azimuthal_filter;
}

std::string
AzimuthalAngleFilter::binName(unsigned int bin_index) const
{
  return "omega" + Moose::stringify(bin_index + 1);
}
#endif
