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

#include "NekSidePostprocessor.h"

/**
 * Compute the integral of pressure over a surface, multiplied by the dot product of
 * the surface unit normal \f$\hat{n}\f$ with a user-specified direction,
 * \f$\hat{d}\f$, or \f$\int_\Gamma -P\hat{n}\cdot\hat{d}d\Gamma\f$, where
 * \f$\Gamma\f$ is the boundary.
 *
 * Note that this calculation is done directly on the mesh that nekRS solves on,
 * _not_ the mesh created for solution transfer in NekRSMesh.
 */
class NekPressureSurfaceForce : public NekSidePostprocessor
{
public:
  static InputParameters validParams();

  NekPressureSurfaceForce(const InputParameters & parameters);

  virtual Real getValue() override;

protected:
  /// Unit direction with which to dot the surface normal
  Point _direction;
};
