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

#include "BinUtility.h"

namespace bin_utility {

unsigned int linearBin(const Real & value, const std::vector<Real> & bounds)
{
  // This finds the first entry in the vector that is larger than what we're looking for
  std::vector<Real>::const_iterator one_higher = std::upper_bound(bounds.begin(), bounds.end(), value);

  if (one_higher == bounds.end())
    return static_cast<unsigned int>(bounds.size() - 2);
  else if (one_higher == bounds.begin())
    return 0;
  else
    return static_cast<unsigned int>(std::distance(bounds.begin(), one_higher - 1));
}

Real midpoint(const unsigned int & bin, const std::vector<Real> & bounds)
{
  return 0.5 * (bounds[bin] + bounds[bin + 1]);
}

} // end namespace bin_utility
