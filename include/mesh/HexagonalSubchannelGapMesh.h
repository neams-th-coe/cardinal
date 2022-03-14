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

#include "HexagonalSubchannelMeshBase.h"

/**
 * Mesh of the gaps in a triangular lattice of pins enclosed in a hexagonal duct;
 * this mesh should ONLY be used for visualization purposes - there is no
 * node connectivity, so you cannot solve any continuous finite element
 * problems on this mesh (nor it is recommended because the element creation
 * pays no attention to normal physics requirements/recommendation, like resolving near
 * boundaries or using near-equal element sizes).
 */
class HexagonalSubchannelGapMesh : public HexagonalSubchannelMeshBase
{
public:
  static InputParameters validParams();

  HexagonalSubchannelGapMesh(const InputParameters & parameters);

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
  void addQuadElem(const Point & pt1,
                   const Point & pt2,
                   const Real & zmin,
                   const Real & zmax,
                   const unsigned int & id);

  /// Number of axial cells to build
  const unsigned int & _n_axial;

  /// Height of assembly
  const Real & _height;

  /// Subdomain ID to set for the interior gaps
  const SubdomainID & _interior_id;

  /// Subdomain ID to set for the peripheral gaps
  const SubdomainID & _peripheral_id;

  /// Pin or side indices corresponding to the gaps
  const std::vector<std::pair<int, int>> & _gap_indices;
};
