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

#ifdef ENABLE_NEK_COUPLING

#include "NekSideSpatialBinUserObject.h"
#include "PlaneSpatialBinUserObject.h"

InputParameters
NekSideSpatialBinUserObject::validParams()
{
  InputParameters params = NekVolumeSpatialBinUserObject::validParams();
  params.addRequiredParam<std::vector<int>>("boundary",
                                            "Boundary ID(s) over which to compute the bin values");
  return params;
}

NekSideSpatialBinUserObject::NekSideSpatialBinUserObject(const InputParameters & parameters)
  : NekVolumeSpatialBinUserObject(parameters), _boundary(getParam<std::vector<int>>("boundary"))
{
  int first_invalid_id, n_boundaries;
  bool valid_ids = nekrs::validBoundaryIDs(_boundary, first_invalid_id, n_boundaries);

  if (!valid_ids)
    mooseError("Invalid 'boundary' entry: ",
               first_invalid_id,
               "\n\n"
               "NekRS assumes the boundary IDs are ordered contiguously beginning at 1. "
               "For this problem, NekRS has ",
               n_boundaries,
               " boundaries.\n"
               "Did you enter a valid 'boundary' for '" +
                   name() + "'?");
}

Point
NekSideSpatialBinUserObject::nekPoint(const int & local_elem_id,
                                      const int & local_face_id,
                                      const int & local_node_id) const
{
  if (_map_space_by_qp)
    return nekrs::gllPointFace(local_elem_id, local_face_id, local_node_id);
  else
    return nekrs::centroidFace(local_elem_id, local_face_id);
}

#endif
