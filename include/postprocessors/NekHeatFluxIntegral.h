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

/**
 * Compute the integral of the heat flux over a boundary
 * in the nekRS mesh, \f$\int_\Gamma -k\nabla T\cdot\hat{n} d\Gamma\f$, where \f$T\f$ is the
 * temperature, \f$k\f$ is the thermal conductivity,
 * \f$\hat{n}\f$ is the unit normal, and \f$\Gamma\f$ is the boundary.
 *
 * Note that this calculation is done directly on the mesh that nekRS solves on,
 * _not_ the mesh created for solution transfer in NekRSMesh.
 *
 * This heat flux calculation directly integrates the heat flux in nekRS - we
 * therefore do not expect this value to match the imposed heat flux at the start
 * of the nekRS time step (i.e. that we interpolated) because those heat fluxes are
 * separated in time.
 *
 * Also very important to note is that the heat flux measured on a boundary that
 * has GLL points also on a Dirichlet boundary will _never_ match the requested
 * imposed heat flux because Dirichlet boundary conditions win (note that this is
 * slightly different from 'Neumann BCs are only weakly imposed').
 */
class NekHeatFluxIntegral : public NekSidePostprocessor
{
public:
  static InputParameters validParams();

  NekHeatFluxIntegral(const InputParameters & parameters);

  virtual Real getValue() override;
};

