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

#include "NekVolumeSpatialBinUserObject.h"

/**
 * Class that performs various postprocessing operations on the
 * NekRS solution with a spatial binning formed as the product of
 * various volume bins with a boundary in the NekRS mesh
 */
class NekSideSpatialBinUserObject : public NekVolumeSpatialBinUserObject
{
public:
  static InputParameters validParams();

  NekSideSpatialBinUserObject(const InputParameters & parameters);

  /**
   * Get the point at which to evaluate the user object
   * @param[in] local_elem_id local element ID on the Nek rank
   * @param[in] local_face_id local face ID on the element
   * @param[in] local_node_id local node ID on the element
   * @return point, in dimensional form
   */
  Point
  nekPoint(const int & local_elem_id, const int & local_face_id, const int & local_node_id) const;

protected:
  /// Boundaries over which to evaluate the user object, on the NekRS mesh
  const std::vector<int> & _boundary;
};
