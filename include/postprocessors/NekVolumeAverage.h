//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "NekVolumeIntegral.h"

class NekVolumeAverage;

template <>
InputParameters validParams<NekVolumeAverage>();

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

protected:
  /// Volume by which to normalize
  Real _volume;
};
