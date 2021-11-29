#pragma once

#include "NekFieldPostprocessor.h"

/**
 * Compute the integral of a specified quantity over the scalar
 * nekRS mesh, \f$\int_\Omega f d\Omega\f$, where \f$f\f$ is the
 * field to be integrated, and \f$\Omega\f$ is the volume.
 *
 * Note that this calculation is done directly on the mesh that nekRS solves on,
 * _not_ the mesh created for solution transfer in NekRSMesh.
 */
class NekVolumeIntegral : public NekFieldPostprocessor
{
public:
  static InputParameters validParams();

  NekVolumeIntegral(const InputParameters & parameters);

  virtual Real getValue() override;

protected:
  /// Volume by which to normalize
  Real _volume;
};
