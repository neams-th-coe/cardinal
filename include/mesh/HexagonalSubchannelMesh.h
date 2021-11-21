#pragma once

#include "HexagonalSubchannelMeshBase.h"

class HexagonalSubchannelMesh;

template <>
InputParameters validParams<HexagonalSubchannelMesh>();

/**
 * Mesh of a triangular lattice of pins enclosed in a hexagonal duct;
 * this mesh should ONLY be used for visualization purposes - there is no
 * node connectivity, so you cannot solve any continuous finite element
 * problems on this mesh (nor it is recommended because the element creation
 * pays no attention to normal physics requirements/recommendation, like resolving near
 * boundaries or using near-equal element sizes).
 */
class HexagonalSubchannelMesh : public HexagonalSubchannelMeshBase
{
public:
  static InputParameters validParams();

  HexagonalSubchannelMesh(const InputParameters & parameters);

  virtual std::unique_ptr<MooseMesh> safeClone() const override;

  virtual void buildMesh() override;

protected:
  /**
   * Get the node positions for a single, upwards-facing, interior subchannel
   * with a centroid at (0, 0, 0)
   */
  void getInteriorPoints();

  /**
   * Get the node positions for a single, downwards-facing, edge subchannel
   * with a centroid at (0, 0, 0)
   */
  void getEdgePoints();

  /**
   * Get the node positions for a single, upper-right, corner subchannel with a
   * centroid at (0, 0, 0)
   */
  void getCornerPoints();

  /**
   * Add a prism6 element for given points in triangle and between two axial planes
   * @param[in] pt1 point in triangle
   * @param[in] pt2 point in triangle
   * @param[in] pt3 point in triangle
   * @param[in] zmin minimum z-coordinate for layer
   * @param[in] zmax maximum z-coordinate for layer
   * @param[in] id subdomain ID
   */
  void addPrismElem(const Point & pt1, const Point & pt2, const Point & pt3, const Real & zmin, const Real & zmax,
    const SubdomainID & id);

  /**
   * Add a tri3 element for given points in triangle on a given axial plane
   * @param[in] pt1 point in triangle
   * @param[in] pt2 point in triangle
   * @param[in] pt3 point in triangle
   * @param[in] z z-coordinate for layer
   * @param[in] id subdomain ID
   */
  void addTriElem(const Point & pt1, const Point & pt2, const Point & pt3, const Real & z, const SubdomainID & id);

  /// Number of nodes on each pin's quarter circumference
  const unsigned int & _theta_res;

  /// Number of nodes on each gap
  const unsigned int & _gap_res;

  /// Number of axial cells to build
  const unsigned int & _n_axial;

  /// Height of assembly
  const Real & _height;

  /// Subdomain ID to set for the interior channels
  const SubdomainID & _interior_id;

  /// Subdomain ID to set for the edge channels
  const SubdomainID & _edge_id;

  /// Subdomain ID to set for the corner channels
  const SubdomainID & _corner_id;

  /**
   * Whether to create a volume mesh (such that the volumes of the subchannels are represented)
   * or a surface mesh (which will create a 2-D mesh on n_axial + 1 planes through the assembly).
   */
  const bool & _volume_mesh;

  /// Node positions for a single upward-facing interior subchannel with a centroid at (0, 0, 0)
  std::vector<Point> _interior_points;

  /// Node positions for a single upward-facing edge subchannel with a centroid at (0, 0, 0)
  std::vector<Point> _edge_points;

  /// Node positions for a single, top-right, corner subchannel with a centroid at (0, 0, 0)
  std::vector<Point> _corner_points;

  /// Number of elements per interior channel
  int _elems_per_interior;

  /// Number of elements per edge channel
  int _elems_per_edge;

  /// Number of elements per corner channel
  int _elems_per_corner;
};
