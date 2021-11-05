#pragma once

#include "SpatialBinUserObject.h"
#include "HexagonalLatticeUtility.h"

/**
 * Class that bins spatial coordinates into a hexagonal subchannel gap discretization
 * with unique bins for each gap.
 */
class HexagonalSubchannelGapBin : public SpatialBinUserObject
{
public:
  static InputParameters validParams();

  HexagonalSubchannelGapBin(const InputParameters & parameters);

  virtual const unsigned int bin(const Point & p) const override;

  virtual const unsigned int num_bins() const override;

  virtual const std::vector<unsigned int> directions() const override { return _directions; }

  virtual bool isSideBinning() const override { return true; }

  virtual const Real distanceFromGap(const Point & point, const unsigned int & gap_index) const {
    return _hex_lattice->distanceFromGap(point, gap_index); }

  virtual unsigned int gapIndex(const Point & point) const { return _hex_lattice->gapIndex(point); }

  virtual void gapIndexAndDistance(const Point & point, unsigned int & index,
    Real & distance) const { return _hex_lattice->gapIndexAndDistance(point, index, distance); }

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
  std::unique_ptr<HexagonalLatticeUtility> _hex_lattice;

  /// Directions along which the bin defines points
  std::vector<unsigned int> _directions;
};
