//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "NekSideIntegral.h"
#include "CardinalEnums.h"

class NekMassFluxWeightedSideIntegral;

template <>
InputParameters validParams<NekMassFluxWeightedSideIntegral>();

/**
 * Compute the integral of a specified quantity over the boundaries
 * in the nekRS mesh, weighted by the mass flux,
 * \f$\int_\Gamma \rho_f\vec{V}\cdot\hat{n} fd\Gamma\f$, where \f$f\f$ is the
 * field to be integrated, \f$\Gamma\f$ is the boundary, and
 * \f$\rho_f\vec{V}\cdot\hat{n}\f$ is the mass flux.
 *
 * Note that this calculation is done directly on the mesh that nekRS solves on,
 * _not_ the mesh created for solution transfer in NekRSMesh.
 */
class NekMassFluxWeightedSideIntegral : public NekSideIntegral
{
public:
  static InputParameters validParams();

  NekMassFluxWeightedSideIntegral(const InputParameters & parameters);

  virtual Real getValue() override;
};

