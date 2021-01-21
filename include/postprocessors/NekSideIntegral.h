//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "NekSidePostprocessor.h"
#include "CardinalEnums.h"

class NekSideIntegral;

template <>
InputParameters validParams<NekSideIntegral>();

/**
 * Compute the integral of a specified quantity over the boundaries
 * in the nekRS mesh, \f$\int_\Gamma f d\Gamma\f$, where \f$f\f$ is the
 * field to be integrated, and \f$\Gamma\f$ is the boundary.
 *
 * Note that this calculation is done directly on the mesh that nekRS solves on,
 * _not_ the mesh created for solution transfer in NekRSMesh.
 */
class NekSideIntegral : public NekSidePostprocessor
{
public:
  static InputParameters validParams();

  NekSideIntegral(const InputParameters & parameters);

  virtual Real getValue() override;

protected:
  /// integrand of the surface integral
  const field::NekFieldEnum _field;
};

