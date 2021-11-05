#pragma once

#include "MooseMesh.h"
#include "HexagonalLatticeUtility.h"

class HexagonalSubchannelGapMesh;

template <>
InputParameters validParams<HexagonalSubchannelGapMesh>();

/**
 * Mesh of the gaps in a triangular lattice of pins enclosed in a hexagonal duct;
 * this mesh should ONLY be used for visualization purposes - there is no
 * node connectivity, so you cannot solve any continuous finite element
 * problems on this mesh (nor it is recommended because the element creation
 * pays no attention to normal physics requirements/recommendation, like resolving near
 * boundaries or using near-equal element sizes).
 */
class HexagonalSubchannelGapMesh : public MooseMesh
{
public:
  static InputParameters validParams();

  HexagonalSubchannelGapMesh(const InputParameters & parameters);
  HexagonalSubchannelGapMesh(const HexagonalSubchannelGapMesh & /* other_mesh */) = default;

  HexagonalSubchannelGapMesh & operator=(const HexagonalSubchannelGapMesh & other_mesh) = delete;

  virtual std::unique_ptr<MooseMesh> safeClone() const override;

  virtual void buildMesh() override;

protected:

  /**
   * Add a QUAD4 element
   * @param[in] pt1 corner point on the z=0 plane
   * @param[in] pt2 corner point on the z=0 plane
   * @param[in] zmin lower z coordinate for the element
   * @param[in] zmax upper z coordinate for the element
   * @param[in] id element subdomain ID
   */
  void addQuadElem(const Point & pt1, const Point & pt2, const Real & zmin, const Real & zmax, const unsigned int & id);

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

  /// Number of axial cells to build
  const unsigned int & _n_axial;

  /// Height of assembly
  const Real & _height;

  /// Subdomain ID to set for the interior gaps
  const SubdomainID & _interior_id;

  /// Subdomain ID to set for the peripheral gaps
  const SubdomainID & _peripheral_id;

  /// Utility providing hexagonal lattice geometry calculations
  HexagonalLatticeUtility _hex_lattice;

  /// Coordinates for the pin centers
  const std::vector<Point> & _pin_centers;

  /// Pin or side indices corresponding to the gaps
  const std::vector<std::pair<int, int>> & _gap_indices;

  /// Element ID
  int _elem_id_counter;

  /// Node ID
  int _node_id_counter;
};
