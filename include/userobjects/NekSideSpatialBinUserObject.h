#pragma once

#include "NekSpatialBinUserObject.h"
#include "SideSpatialBinUserObject.h"

/**
 * Class that performs various postprocessing operations on the
 * NekRS solution with a spatial binning formed as the product
 * of two bin distributions, one a surface distribution and the other
 * a volume distribution.
 */
class NekSideSpatialBinUserObject : public NekSpatialBinUserObject
{
public:
  static InputParameters validParams();

  NekSideSpatialBinUserObject(const InputParameters & parameters);

  virtual Real distanceFromGap(const Point & point, const unsigned int & gap_index) const;

  virtual unsigned int gapIndex(const Point & point) const;

  virtual void gapIndexAndDistance(const Point & point, unsigned int & index, Real & distance) const;

protected:
  /// Width of region enclosing gap for which points contribute to gap integral
  const Real & _gap_thickness;

  /// Width of region enclosing gap, in nondimensional form
  const Real _nondimensional_gap_thickness;

  /// The user object providing the side binning
  const SideSpatialBinUserObject * _side_bin;

  /// The index into the _bins that represents the side bin object
  unsigned int _side_index;
};
