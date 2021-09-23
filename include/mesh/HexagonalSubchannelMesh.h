#pragma once

#include "MooseMesh.h"
#include "HexagonalLatticeUtility.h"

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
class HexagonalSubchannelMesh : public MooseMesh
{
public:
  static InputParameters validParams();

  HexagonalSubchannelMesh(const InputParameters & parameters);
  HexagonalSubchannelMesh(const HexagonalSubchannelMesh & /* other_mesh */) = default;

  HexagonalSubchannelMesh & operator=(const HexagonalSubchannelMesh & other_mesh) = delete;

  virtual std::unique_ptr<MooseMesh> safeClone() const override;

  virtual void buildMesh() override;

protected:
  /**
   * Rotate a point counterclockwise about the z axis
   * @param[in] p point
   * @param[in] theta angle (radians) by which to rotate
   * @return rotated point
   */
  const Point rotatePoint(const Point & p, const Real & theta) const;

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
   * Add the elements for an interior channel
   * @param[in] centroid centroid for interior channel
   * @param[in] zmin minimum z-coordinate for axial layer
   * @param[in] zmax maximum z-coordinate for axial layer
   * @param[in] rotation optional rotation to apply to elements
   */
  void addInteriorChannel(const Point & centroid, const Real & zmin, const Real & zmax, const Real & rotation);

  /**
   * Add the elements for an edge channel
   * @param[in] centroid centroid for edge channel
   * @param[in] zmin minimum z-coordinate for axial layer
   * @param[in] zmax maximum z-coordinate for axial layer
   * @param[in] rotation optional rotation to apply to elements
   */
  void addEdgeChannel(const Point & centroid, const Real & zmin, const Real & zmax, const Real & rotation);

  /**
   * Add the elements for a corner channel
   * @param[in] centroid centroid for corner channel
   * @param[in] zmin minimum z-coordinate for axial layer
   * @param[in] zmax maximum z-coordinate for axial layer
   * @param[in] rotation optional rotation to apply to elements
   */
  void addCornerChannel(const Point & centroid, const Real & zmin, const Real & zmax, const Real & rotation);

  /**
   * Add all interior channels in a given axial layer
   * @param[in] zmin minimum z-coordinate for layer
   * @param[in] zmax maximum z-coordinate for layer
   */
  void addInteriorChannels(const Real & zmin, const Real & zmax);

  /**
   * Add all edge channels in a given axial layer
   * @param[in] zmin minimum z-coordinate for layer
   * @param[in] zmax maximum z-coordinate for layer
   */
  void addEdgeChannels(const Real & zmin, const Real & zmax);

  /**
   * Add all corner channels in a given axial layer
   * @param[in] zmin minimum z-coordinate for layer
   * @param[in] zmax maximum z-coordinate for layer
   */
  void addCornerChannels(const Real & zmin, const Real & zmax);

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

  /// Bundle pitch (distance across bundle measured flat-to-flat on the inside of the duct)
  const Real & _bundle_pitch;

  /// Pin pitch
  const Real & _pin_pitch;

  /// Pin diameter
  const Real & _pin_diameter;

  /// Total number of rings of pins
  const unsigned int & _n_rings;

  /// Vertical axis of the bundle along which the pins are aligned
  const unsigned int _axis;

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

  /// Utility providing hexagonal lattice geometry calculations
  HexagonalLatticeUtility _hex_lattice;

  /// Coordinates for the pin centers
  const std::vector<Point> & _pin_centers;

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

  /// Element ID
  int _elem_id_counter;

  /// Node ID
  int _node_id_counter;

  static const Real SIN30;

  static const Real COS30;

  /// Number of nodes per prism6 element
  static const unsigned int NODES_PER_PRISM;

  /// Number of sides in a hexagon
  static const unsigned int NUM_SIDES;
};
