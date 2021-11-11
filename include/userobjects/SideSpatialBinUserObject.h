#pragma once

#include "SpatialBinUserObject.h"

/**
 * Class that provides a side bin index given a spatial coordinate
 */
class SideSpatialBinUserObject : public SpatialBinUserObject
{
public:
  static InputParameters validParams();

  SideSpatialBinUserObject(const InputParameters & parameters);

  /**
   * Distance between a point and a gap
   * @param[in] point point
   * @param[in] gap_index gap index
   */
  virtual Real distanceFromGap(const Point & point, const unsigned int & gap_index) const = 0;

  /**
   * Gap index closest to the point
   * @param[in] point point
   * @return gap index
   */
  virtual unsigned int gapIndex(const Point & point) const = 0;

  /**
   * Distance between a point and a gap
   * @param[in] point point
   * @param[out] index index of the gap that the point is closest to
   * @param[out] distance distance from point to the closest gap
   */
  virtual void gapIndexAndDistance(const Point & point, unsigned int & index, Real & distance) const = 0;

  /**
   * Get the unit normals for each gap
   * @return gap unit normals
   */
  virtual const std::vector<Point> & gapUnitNormals() const = 0;
};
