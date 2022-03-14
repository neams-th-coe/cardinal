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

#include "NekSpatialBinUserObject.h"
#include "PlaneSpatialBinUserObject.h"

/**
 * Class that performs various postprocessing operations on the
 * NekRS solution with a spatial binning formed as the product
 * of two bin distributions, one a surface distribution and the other
 * a volume distribution.
 */
class NekPlaneSpatialBinUserObject : public NekSpatialBinUserObject
{
public:
  static InputParameters validParams();

  NekPlaneSpatialBinUserObject(const InputParameters & parameters);

  virtual Real distanceFromGap(const Point & point, const unsigned int & gap_index) const;

  virtual unsigned int gapIndex(const Point & point) const;

  virtual void
  gapIndexAndDistance(const Point & point, unsigned int & index, Real & distance) const;

protected:
  /// Width of region enclosing gap for which points contribute to gap integral
  const Real & _gap_thickness;

  /// The user object providing the side binning
  const PlaneSpatialBinUserObject * _side_bin;

  /// The index into the _bins that represents the side bin object
  unsigned int _side_index;
};
