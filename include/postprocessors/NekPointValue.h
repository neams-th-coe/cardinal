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

#include "NekFieldPostprocessor.h"

/**
 * Interpolate a NekRS solution field onto a point, using NekRS's
 * pointInterpolation. The provided point does not need to be a grid point;
 * interpolation happens on GPU.
 *
 * NOTE: NekRS does not provide an interface to capture any errors if a point
 * provided is not actually contained. They only print to the console. Any
 * point specified, but which is outside the mesh, will silently return zero.
 */
class NekPointValue : public NekFieldPostprocessor
{
public:
  static InputParameters validParams();

  NekPointValue(const InputParameters & parameters);

  virtual Real getValue() const override;

  virtual void execute() override;

protected:
  /// Point where the field will be evaluated
  const Point & _point;

  /// The value of the variable at the desired location
  Real _value;
};
