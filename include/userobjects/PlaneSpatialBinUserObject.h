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

#include "SpatialBinUserObject.h"

/**
 * Class that provides a side bin index given a spatial coordinate
 */
class PlaneSpatialBinUserObject : public SpatialBinUserObject
{
public:
  static InputParameters validParams();

  PlaneSpatialBinUserObject(const InputParameters & parameters);

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
  virtual void
  gapIndexAndDistance(const Point & point, unsigned int & index, Real & distance) const = 0;

  /**
   * Get the unit normals for each gap
   * @return gap unit normals
   */
  virtual const std::vector<Point> & gapUnitNormals() const = 0;

  /**
   * Apply an additional normalization factor to the bin value
   * @param[in] bin bin index
   * @return multiplicative value to apply
   */
  virtual Real adjustBinValue(const unsigned int & bin) const { return 1.0; }
};
