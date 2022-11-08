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

#include "libmesh/cell_hex8.h"
#include "libmesh/cell_hex27.h"

/**
 * Mesh generator for converting a HEX8 mesh to a HEX20 mesh, with optional
 * preservation of circular edges by moving the mid-edge nodes to form a
 * circular sideset in the plane perpendicular to the 'axis'.
 */
class Hex20Generator : public NekMeshGenerator
{
public:
  static InputParameters validParams();

  Hex20Generator(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

  virtual void checkElementType(std::unique_ptr<MeshBase> & mesh) override;

  virtual unsigned int getFaceNode(const unsigned int & primary_face) const override;

  virtual std::pair<unsigned int, unsigned int> pairedNodesAboutMidPoint(const unsigned int & node_id) const override;

  virtual bool isCornerNode(const unsigned int & node) const override
  {
    return node < Hex8::num_nodes;
  }

  virtual const std::vector<unsigned int> nodesOnFace(const unsigned int & face) const override
  {
    const auto & side_map = Hex27::side_nodes_map[face];
    std::vector<unsigned int> nodes;
    nodes.assign(std::begin(side_map), std::end(side_map));
    return nodes;
  }
};
