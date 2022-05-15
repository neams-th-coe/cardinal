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
class Hex20Generator : public MeshGenerator
{
public:
  static InputParameters validParams();

  Hex20Generator(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

  /**
   * Get the node index pertaining to a given point
   * @param[in] elem element
   * @param[in] pt point of interest
   * @return node index corresponding to the point
   */
  unsigned int getNodeIndex(const Elem * elem, const Point & pt) const;

  /**
   * Get the node "paired" to the present node "across" the face
   * @param[in] node_id node ID on primary face
   * @param[in] face_id ID of primary face
   * @return node ID "across" that face to the other side of the Hex27
   */
  unsigned int pairedFaceNode(const unsigned int & node_id, const unsigned int & face_id) const;

  /**
   * For an element on the 'boundary' of interest, move its nodes, as well as those
   * of any boundary layers
   * @param[in] elem element on the primary 'boundary'
   * @param[in] boundary_index index of the 'boundary'
   * @param[in] primary_face face ID of the element on the 'boundary'
   */
  void moveElem(Elem * elem, const unsigned int & boundary_index, const unsigned int & primary_face);

  /**
   * Get a pointer to the next element in the boundary layer
   * @param[in] elem current element in the boundary layer
   * @param[in] primary_face face of the element that touches the "next" element
   * @param[out] next_touching_face face ID of the "next" element
   */
  const Elem * getNextLayerElem(const Elem & elem, const unsigned int & touching_face,
    unsigned int & next_touching_face) const;

  /**
   * Get the two nodes to average coordinates for given a side-node id
   * @param[in] node_id mid-point side node ID
   * @return two nodes on the corners to average to get new coordinates
   */
  std::pair<unsigned int, unsigned int> pairedNodesAboutMidPoint(const unsigned int & node_id) const;

  /**
   * Find the origin closest to the given point
   * @param[in] index boundary index to look for origins
   * @param[in] pt point of interest
   * @return closest origin
   */
  Point getClosestOrigin(const unsigned int & index, const Point & pt) const;

  /**
   * Project a point onto the (x,y) plane (perpendicular to the 'axis'), relative to the origin
   * @param[in] origin origin
   * @param[in] pt point to project
   * @return projected point
   */
  Point projectPoint(const Point & origin, const Point & pt) const;

  /**
   * Get the boundary ID pertaining to a boundary name, and check for validity
   * @param[in] name boundary to check
   * @param[in] mesh mesh
   * @return boundary ID corresponding to name
   */
  BoundaryID getBoundaryID(const BoundaryName & name, const MeshBase & mesh) const;

  /**
   * Check that the mesh is centered on (0, 0) (in the plane perpendicular to the
   * circular surface).
   * @param[in] mesh mesh to check
   */
  void checkOrigin(const MeshBase & mesh) const;

  /**
   * Whether a node on a face if a corner node
   * @param[in] node node index
   * @return whether node is a corner node
   */
  bool isCornerNode(const unsigned int & node) const
  {
    return node < Hex8::num_nodes;
  }

  /**
   * Adjust the point to which a face node should move to fit onto the circle
   * @param[in] node_id node ID
   * @param[in] elem element of interest
   * @return adjustment made to point, for use when shifting other boundary layers
   */
  Point adjustPointToCircle(const unsigned int & node_id, Elem * elem, const Real & radius, const Point & origin) const;

  /**
   * Get the node indices on a given face
   * @param[in] face face index
   * @return node indices on that face
   */
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

  /// Whether sidesets will be moved to match circular surfaces
  const bool _has_moving_boundary;

  /// Sideset IDs to move to fit the circular surface
  std::vector<BoundaryID> _moving_boundary;

  /// Radii of the circular surface(s)
  std::vector<Real> _radius;

  /// Origins of the circular surface(s)
  std::vector<std::vector<Real>> _origin;

  /// For each face, the mid-side nodes to be adjusted
  std::vector<std::vector<unsigned int>> _side_ids;

  /// For each face, the corner node indices
  std::vector<std::vector<unsigned int>> _corner_nodes;

  /// Original boundary names and IDs
  std::map<BoundaryID, BoundaryName> _boundary_id_to_name;

  /// Boundaries to rebuild in the new mesh
  std::set<BoundaryID> _boundaries_to_rebuild;

  /// Number of layers to sweep on each boundary
  std::vector<unsigned int> _layers;

  /// For each face, the node pairing "across" that face to the other side of a Hex27
  std::vector<std::vector<std::pair<unsigned int, unsigned int>>> _across_pair;

  /// For each face, the paired face "across" to the other side of a Hex27
  std::vector<unsigned int> _across_face;
};
