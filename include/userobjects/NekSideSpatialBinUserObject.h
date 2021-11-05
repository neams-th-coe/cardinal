#pragma once

#include "NekSpatialBinUserObject.h"

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

  virtual const Real distanceFromGap(const Point & point, const unsigned int & gap_index) const
    { return _side_bin->distanceFromGap(point, gap_index); }

  virtual unsigned int gapIndex(const Point & point) const { return _side_bin->gapIndex(point); }

  virtual void gapIndexAndDistance(const Point & point, unsigned int & index,
    Real & distance) const { return _side_bin->gapIndexAndDistance(point, index, distance); }

protected:
  /// Width of region enclosing gap for which points contribute to gap integral
  const Real & _gap_thickness;

  /// The user object providing the side binning
  const SpatialBinUserObject * _side_bin;
};
