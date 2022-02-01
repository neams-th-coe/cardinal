#pragma once

/// Store the geometry and parallel information related to the surface mesh coupling
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
  int * element;

  // element-local face IDs on the boundary of interest (for all ranks)
  int * face;

  // problem-global boundary ID for each element (for all ranks)
  int * boundary_id;

  // process owning each face (for all faces)
  int * process;

  // number of faces owned by each process
  int * counts;

  // number of coupling elements owned by this process
  int n_faces;

  // total number of coupling elements
  int total_n_faces;

  // offset into the element, face, and process arrays where this rank's data begins
  int offset;
};
