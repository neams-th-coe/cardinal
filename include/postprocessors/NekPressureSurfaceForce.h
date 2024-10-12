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
 * Compute the integral of pressure force that the fluid exerts on a surface,
 * or \f$\int_\Gamma Pn_id\Gamma\f$, where $n_i$ is either the $x$/$y$/$z$
 * direction (or the magnitude of the entire force, if the component is 'total').
 *
 * Note that this calculation is done directly on the mesh that nekRS solves on,
 * _not_ the mesh created for solution transfer in NekRSMesh.
 */
class NekPressureSurfaceForce : public NekSidePostprocessor
{
public:
  static InputParameters validParams();

  NekPressureSurfaceForce(const InputParameters & parameters);

  virtual Real getValue() const override;

protected:
  /// Component of force to compute
  const MooseEnum & _component;
};
