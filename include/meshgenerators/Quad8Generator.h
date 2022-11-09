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

#include "NekMeshGenerator.h"

#include "libmesh/face_quad4.h"
#include "libmesh/face_quad8.h"
#include "libmesh/face_quad9.h"

/**
 * Mesh generator for converting a QUAD9 mesh to a QUAD8 mesh, with optional
 * preservation of circular edges by moving the mid-edge nodes to form a
 * circular sideset in the plane perpendicular to the 'axis'.
 */
class Quad8Generator : public NekMeshGenerator
{
public:
  static InputParameters validParams();

  Quad8Generator(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;
};
