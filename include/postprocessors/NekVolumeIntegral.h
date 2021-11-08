//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "NekPostprocessor.h"
#include "CardinalEnums.h"

class NekVolumeIntegral;

template <>
InputParameters validParams<NekVolumeIntegral>();

/**
 * Compute the integral of a specified quantity over the scalar
 * nekRS mesh, \f$\int_\Omega f d\Omega\f$, where \f$f\f$ is the
 * field to be integrated, and \f$\Omega\f$ is the volume.
 *
 * Note that this calculation is done directly on the mesh that nekRS solves on,
 * _not_ the mesh created for solution transfer in NekRSMesh.
 */
class NekVolumeIntegral : public NekPostprocessor
{
public:
  static InputParameters validParams();

  NekVolumeIntegral(const InputParameters & parameters);

  virtual void initialize() override {}
  virtual void execute() override {}

  virtual Real getValue() override;

protected:
  /// integrand of the surface integral
  const field::NekFieldEnum _field;

  /// Volume by which to normalize
  Real _volume;
};
