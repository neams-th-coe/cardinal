#pragma once

#include "CardinalUtils.h"

class NekVolumeCoupling
{
public:
  ~NekVolumeCoupling()
  {
    freePointer(element);
    freePointer(process);
    freePointer(boundary);
    freePointer(counts);
    freePointer(n_faces_on_boundary);
  }

  /**
   * nekRS process owning the global element in the data transfer mesh
   * @param[in] elem_id element ID
   * @return nekRS process ID
   */
  int processor_id(const int elem_id) const { return process[elem_id]; }

  // process-local element IDS (for all elements)
  int * element = nullptr;

  // process owning each element (for all elements)
  int * process = nullptr;

  // sideset IDs corresponding to the faces of each element (for all elements)
  int * boundary = nullptr;

  // number of elements owned by each process
  int * counts = nullptr;

  // number of faces on a boundary of interest for each element
  int * n_faces_on_boundary = nullptr;

  // number of coupling elements owned by this process
  int n_elems = 0;

  // total number of coupling elements
  int total_n_elems = 0;
};
