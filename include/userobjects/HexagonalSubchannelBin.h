#pragma once

#include "SpatialBinUserObject.h"
#include "HexagonalLatticeUtility.h"

/**
 * Class that bins spatial coordinates into a hexagonal subchannel discretization
 * with unique bins for each channel.
 */
class HexagonalSubchannelBin : public SpatialBinUserObject
{
public:
  static InputParameters validParams();

  HexagonalSubchannelBin(const InputParameters & parameters);

  virtual const unsigned int bin(const Point & p) const override;

  virtual const unsigned int num_bins() const override;

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
  const unsigned int & _axis;

  /// Underlying utility providing hexagonal lattice capabilities
  std::unique_ptr<HexagonalLatticeUtility> _hex_lattice;
};
