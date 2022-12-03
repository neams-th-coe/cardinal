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

#include "CardinalUtils.h"

/**
 * Store the geometry and parallel information related to the surface mesh coupling.
 * Unless otherwise noted, all information in here is indexed according to the
 * NekRS mesh, and *not* the mesh mirror (this is only relevant for 'exact' mesh mirrors,
 * where we build N^2 or N^3 MOOSE elements for each NekRS element).
 */
class NekBoundaryCoupling
{
public:
  /**
   * nekRS process owning the global element in the data transfer mesh
   * @param[in] elem_id element ID
   * @return nekRS process ID
   */
  int processor_id(const int elem_id) const { return process[elem_id]; }

  // process-local element IDS on the boundary of interest (for all ranks)
  std::vector<int> element;

  // element-local face IDs on the boundary of interest (for all ranks)
  std::vector<int> face;

  // problem-global boundary ID for each element (for all ranks)
  std::vector<int> boundary_id;

  // process owning each face (for all faces)
  std::vector<int> process;

  // number of faces owned by each process
  std::vector<int> counts;

  // number of MOOSE mirror faces owned by each process
  std::vector<int> mirror_counts;

  // number of coupling elements owned by this process
  int n_faces = 0;

  // total number of coupling elements
  int total_n_faces = 0;

  // offset into the element, face, and process arrays where this rank's data begins
  int offset = 0;
};
