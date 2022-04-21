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

#include "MeshGenerator.h"

#include "libmesh/cell_hex8.h"
#include "libmesh/cell_hex27.h"

/**
 * MeshGenerator for converting a HEX8 mesh to a HEX20 mesh, with optional
 * preservation of circular edges by moving the mid-edge nodes to form a
 * circular sideset in the plane perpendicular to the 'axis'.
 */
class SecondOrderHexGenerator : public MeshGenerator
{
public:
  static InputParameters validParams();

  SecondOrderHexGenerator(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

  bool isCornerNode(const unsigned int & node) const
  {
    return node < Hex8::num_nodes;
  }

  /**
   * Adjust the point to which a face node should move to fit onto the circle
   * @param[in] node_id node ID
   * @param[in] elem element of interest
   */
  void adjustPointToCircle(const unsigned int & node_id, Elem * elem) const;

  const std::vector<unsigned int> nodesOnFace(const unsigned int & face) const
  {
    const auto & side_map = Hex27::side_nodes_map[face];
    std::vector<unsigned int> nodes;
    nodes.assign(std::begin(side_map), std::end(side_map));
    return nodes;
  }

  /**
   * Adjust the mid-point side node to the average of the corners
   * @param[in] node_id index for the node on the side mid-point
   * @param[in] elem element of interest
   * @return average of the two corner nodes corresponding to the node_id mid-side node
   */
  void adjustMidPointNode(const unsigned int & node_id, Elem * elem) const;

  /**
   * Get the node index corresponding to the mid-point side for a given corner node and face
   * @param[in] face_id face ID of the element
   * @param[in] face_node node on given face
   * @return index of the mid-point node for that corner node "perpendicular" from the face
   */
  unsigned int midPointNodeIndex(const unsigned int & face_id, const unsigned int & face_node) const;

protected:
  /// Mesh to modify
  std::unique_ptr<MeshBase> & _input;

  /// Axis of the mesh about which to build the circular surface
  const MooseEnum & _axis;

  /// Sideset IDs to move to fit the circular surface
  const std::vector<BoundaryID> * _boundary;

  /// Radius of the circular surface
  Real _radius;

  /// For each face, the mid-side nodes to be adjusted
  std::vector<std::vector<unsigned int>> _side_ids;

  /// For each face, the corner node indices
  std::vector<std::vector<unsigned int>> _corner_nodes;
};
