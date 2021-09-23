#pragma once

#include "SpatialBinUserObject.h"

/**
 * Class that bins spatial coordinates into a 1-D set of layers in the
 * radial direction.
 */
class RadialBin : public SpatialBinUserObject
{
public:
  static InputParameters validParams();

  RadialBin(const InputParameters & parameters);

  virtual const unsigned int bin(const Point & p) const override;

  virtual const unsigned int num_bins() const override;

protected:
  /// Direction of the vertical axis (x, y, or z)
  const unsigned int _vertical_axis;

  /// Minimum radial coordinate
  const Real & _rmin;

  /// Maximum radial coordinate
  const Real & _rmax;

  /// Number of radial layers
  const unsigned int & _nr;

  /// Growth factor to apply to successive layers
  const Real & _growth_r;

  /// Points defining the bounds of the radial regions
  std::vector<Real> _radial_pts;
};
