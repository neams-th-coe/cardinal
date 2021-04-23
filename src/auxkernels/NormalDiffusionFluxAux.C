//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NormalDiffusionFluxAux.h"
#include "Assembly.h"

registerMooseObject("CardinalApp", NormalDiffusionFluxAux);

template <>
InputParameters
validParams<NormalDiffusionFluxAux>()
{
  InputParameters params = validParams<AuxKernel>();

  params.addRequiredCoupledVar("coupled", "Coupled variable for calculation of the flux");
  params.addRequiredParam<MaterialPropertyName>("diffusivity",
                                                "Material property to use as the 'diffusivity'");

  return params;
}

NormalDiffusionFluxAux::NormalDiffusionFluxAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _diffusivity(getMaterialProperty<Real>("diffusivity")),
    _coupled_gradient(coupledGradient("coupled")),
    _normals(_assembly.normals())
{
  if (!isParamValid("boundary"))
    paramError("boundary", "A boundary must be provided for 'NormalDiffusionFluxAux'!");

  mooseError("The NormalDiffusionFluxAux auxkernel has been replaced by the DiffusionFluxAux auxkernel. "
    "Please replace 'NormalDiffusionFluxAux' with 'DiffusionFluxAux', substitute 'coupled' by 'diffusion_variable', "
    "and include 'component = normal'");
}

Real
NormalDiffusionFluxAux::computeValue()
{
  return -_diffusivity[_qp] * _coupled_gradient[_qp] * _normals[_qp];
}
