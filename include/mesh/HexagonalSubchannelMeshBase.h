#pragma once

#include "MooseMesh.h"
#include "HexagonalLatticeUtility.h"

/**
 * Base class for creating meshes related to subchannel discretizations
 * in a hexagonal lattice.
 */
class HexagonalSubchannelMeshBase : public MooseMesh
{
public:
  static InputParameters validParams();

  HexagonalSubchannelMeshBase(const InputParameters & parameters);
  HexagonalSubchannelMeshBase(const HexagonalSubchannelMeshBase & /* other_mesh */) = default;

  HexagonalSubchannelMeshBase & operator=(const HexagonalSubchannelMeshBase & other_mesh) = delete;

protected:
  /**
   * Rotate a point counterclockwise about the z axis
   * @param[in] p point
   * @param[in] theta angle (radians) by which to rotate
   * @return rotated point
   */
  const Point rotatePoint(const Point & p, const Real & theta) const;

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

  /// Utility providing hexagonal lattice geometry calculations
  HexagonalLatticeUtility _hex_lattice;

  /// Coordinates for the pin centers
  const std::vector<Point> & _pin_centers;

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
