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

#include <vector>

namespace nekrs
{

/**
 * \brief Get the corner indices for the GLL points in a NekRS element on a single face
 *
 * Support the NekRS mesh face has nodes
 *
 * 6 -- 7 -- 8
 * |    |    |
 * 3 -- 4 -- 5
 * |    |    |
 * 0 -- 1 -- 2
 *
 * If 'exact' is false, this method returns {0, 2, 6, 8}. If 'exact' is true,
 * this method returns:
 * - element 0: {0, 1, 3, 4}
 * - element 1: {1, 2, 4, 5}
 * - element 2: {3, 4, 6, 7}
 * - element 3: {4, 5, 7, 8}
 *
 * @param[in] n order of mesh (1 = first-order, 2 = second-order)
 * @param[in] exact whether the MOOSE elements will exactly represent the NekRS mesh
 * @return GLL indices, indexed first by element to build, then second by nodes on that element
 */
std::vector<std::vector<int>> cornerGLLIndices(const int & n, const bool exact);

} // end namespace nekrs
