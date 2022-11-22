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
 * Class that bins spatial coordinates into a 1-D set of layers between
 * the bounding box of the domain.
 */
class LayeredBin : public SpatialBinUserObject
{
public:
  static InputParameters validParams();

  LayeredBin(const InputParameters & parameters);

  virtual unsigned int bin(const Point & p) const override;

  virtual unsigned int num_bins() const override;

protected:
  /// Direction of the bins (x, y, or z)
  const unsigned int _direction;

  /// Number of equal-size layers
  const unsigned int & _num_layers;

  /// Underlying problem
  const SubProblem * _layered_subproblem;

  /// Minimum coordinate in the direction
  Real _direction_min;

  /// Maxium coordinate in the direction
  Real _direction_max;

  /// Bounds of the 1-D layering
  std::vector<Real> _layer_pts;
};
