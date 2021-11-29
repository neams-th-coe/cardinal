#pragma once

#include "NekVolumeIntegral.h"

/**
 * Compute the average of a specified quantity over the volume of
 * the nekRS mesh, \f$\frac{\int_\Omega f d\Omega}{\int_\Omega d\Omega}\f$,
 * where \f$f\f$ is the field to be integrated, and \f$\Omega\f$ is the volume.
 *
 * Note that this calculation is done directly on the mesh that nekRS solves on,
 * _not_ the mesh created for solution transfer in NekRSMesh.
 */
class NekVolumeAverage : public NekVolumeIntegral
{
public:
  static InputParameters validParams();

  NekVolumeAverage(const InputParameters & parameters);

  virtual Real getValue() override;
};
