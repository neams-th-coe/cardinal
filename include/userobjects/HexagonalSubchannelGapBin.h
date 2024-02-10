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

#include "PlaneSpatialBinUserObject.h"
#include "HexagonalLatticeUtils.h"

/**
 * Class that bins spatial coordinates into a hexagonal subchannel gap discretization
 * with unique bins for each gap.
 */
class HexagonalSubchannelGapBin : public PlaneSpatialBinUserObject
{
public:
  static InputParameters validParams();

  HexagonalSubchannelGapBin(const InputParameters & parameters);

  virtual unsigned int bin(const Point & p) const override;

  virtual unsigned int num_bins() const override;

  virtual Real distanceFromGap(const Point & point, const unsigned int & gap_index) const override;

  virtual unsigned int gapIndex(const Point & point) const override;

  virtual void
  gapIndexAndDistance(const Point & point, unsigned int & index, Real & distance) const override;

  virtual const std::vector<Point> & gapUnitNormals() const override
  {
    return _hex_lattice->gapUnitNormals();
  }

protected:
  /// Bundle pitch
  const Real & _bundle_pitch;

  /// Pin pitch
  const Real & _pin_pitch;

  /// Pin diameter
  const Real & _pin_diameter;

  /// Total number of rings of pins
  const unsigned int & _n_rings;

  /// Vertical axis of the bundle along which the pins are aligned
  const unsigned int _axis;

  /// Underlying utility providing hexagonal lattice capabilities
  std::unique_ptr<HexagonalLatticeUtils> _hex_lattice;
};
