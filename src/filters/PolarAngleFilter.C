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

#include "PolarAngleFilter.h"

#include "openmc/tallies/filter_polar.h"

registerMooseObject("CardinalApp", PolarAngleFilter);

InputParameters
PolarAngleFilter::validParams()
{
  auto params = FilterBase::validParams();
  params.addClassDescription("A class which provides a thin wrapper around an OpenMC PolarFilter. "
                             "Bins can either be equally spaced by setting 'num_equal_divisions', "
                             "or a set of polar angles can "
                             "be provided by setting 'polar_angle_boundaries'.");
  params.addRangeCheckedParam<unsigned int>(
      "num_equal_divisions",
      "num_equal_divisions > 0",
      "The number of equally spaces subdivisions of "
      "$[0, \\pi]$ to use if equal subdivisions are desired.");
  params.addParam<std::vector<Real>>("polar_angle_boundaries",
                                     "The polar angle boundaries in $[0, \\pi]$ which must be "
                                     "provided in increasing order. If 0 and "
                                     "$\\pi$ are not included this filter may result in some "
                                     "missed particles during normalization.");

  return params;
}

PolarAngleFilter::PolarAngleFilter(const InputParameters & parameters) : FilterBase(parameters)
{
  if (isParamValid("num_equal_divisions") == isParamValid("polar_angle_boundaries"))
    mooseError("You have either set 'num_equal_divisions' and 'polar_angle_boundaries' or have not "
               "specified a bin option! Please specify either 'num_equal_divisions' or "
               "'polar_angle_boundaries'.");

  if (isParamValid("num_equal_divisions"))
  {
    auto num_angles = getParam<unsigned int>("num_equal_divisions");
    for (unsigned int i = 0; i < num_angles + 1; ++i)
      _polar_angle_bnds.push_back((libMesh::pi / num_angles) * i);
  }

  if (isParamValid("polar_angle_boundaries"))
  {
    _polar_angle_bnds = getParam<std::vector<Real>>("polar_angle_boundaries");

    // Make sure we have at least two boundaries to form bins.
    if (_polar_angle_bnds.size() < 2)
      paramError("polar_angle_boundaries",
                 "At least two polar angles are required to create bins!");

    // Sort the boundaries so they're monotonically decreasing.
    std::sort(_polar_angle_bnds.begin(),
              _polar_angle_bnds.end(),
              [](const Real & a, const Real & b) { return a < b; });

    // Warn the user if there is the possibility of missed particles.
    if (_polar_angle_bnds.front() > 0.0 || _polar_angle_bnds.back() < libMesh::pi)
      mooseWarning(
          "The bin boundaries don't contain one of the following: 0 or pi. Some particles may be "
          "missed during normalization.");
  }

  // Initialize the OpenMC PolarFilter.
  _filter_index = openmc::model::tally_filters.size();

  auto polar_filter = dynamic_cast<openmc::PolarFilter *>(openmc::Filter::create("polar"));
  polar_filter->set_bins(_polar_angle_bnds);
  _filter = polar_filter;
}

std::string
PolarAngleFilter::binName(unsigned int bin_index) const
{
  return "theta" + Moose::stringify(bin_index + 1);
}
#endif
