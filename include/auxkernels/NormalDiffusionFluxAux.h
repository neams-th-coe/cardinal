//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AuxKernel.h"

class NormalDiffusionFluxAux;

template <>
InputParameters validParams<NormalDiffusionFluxAux>();

/**
 * Computes the normal diffusion flux of a coupled variable on a boundary,
 * or \f$-k\nabla T\cdot\hat{n}\f$, where \f$k\f$ is the thermal conductivity
 * (called the 'diffusivity' in this auxkernels' input parameters),
 * \f$T\f$ is the coupled variable, and \f$\hat{n}\f$ is the unit normal vector.
 */
class NormalDiffusionFluxAux : public AuxKernel
{
public:
  NormalDiffusionFluxAux(const InputParameters & parameters);

  virtual ~NormalDiffusionFluxAux() {}

protected:
  virtual Real computeValue();

  /// material property multiplying \f$\nabla T\cdot\hat{n}\f$
  const MaterialProperty<Real> & _diffusivity;

  /// coupled gradient of \f$T\f$
  const VariableGradient & _coupled_gradient;

  /// normals at quadrature points
  const MooseArray<Point> & _normals;
};
