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
 * Store the geometry and parallel information related to the volume mesh coupling.
 * Unless otherwise noted, all information in here is indexed according to the
 * NekRS mesh, and *not* the mesh mirror (this is only relevant for 'exact' mesh mirrors,
 * where we build N^2 or N^3 MOOSE elements for each NekRS element).
 */
class NekVolumeCoupling
{
public:
  /**
   * nekRS process owning the global element in the data transfer mesh
   * @param[in] elem_id element ID
   * @return nekRS process ID
   */
  int processor_id(const int elem_id) const { return process[elem_id]; }

  // process-local element IDS (for all elements)
  std::vector<int> element;

  // process owning each element (for all elements)
  std::vector<int> process;

  // sideset IDs corresponding to the faces of each element (for all elements)
  std::vector<int> boundary;

  // number of elements owned by each process
  std::vector<int> counts;

  // number of MOOSE mirror elements owned by each process
  std::vector<int> mirror_counts;

  // number of faces on a boundary of interest for each element
  std::vector<int> n_faces_on_boundary;

  // number of coupling elements owned by this process
  int n_elems = 0;

  // total number of coupling elements
  int total_n_elems = 0;
};
