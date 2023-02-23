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

#pragma once

#include "Moose.h"

namespace bin_utility {

/**
 * Map value onto a linear binning
 * @param[in] value input value
 * @param[in] bounds bounds of the bins
 * @return bin index
 */
unsigned int linearBin(const Real & value, const std::vector<Real> & bounds);

/**
 * Get the midpoint of a bin
 * @param[in] bin index
 * @param[in] bounds bounds of the bins
 * @return midpoint
 */
Real midpoint(const unsigned int & bin, const std::vector<Real> & bounds);

} // end of namespace bin_utility
