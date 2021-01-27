//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FluxAverageAux.h"
#include "Assembly.h"

registerMooseObject("CardinalApp", FluxAverageAux);

template <>
InputParameters
validParams<FluxAverageAux>()
{
  InputParameters params = validParams<NormalDiffusionFluxAux>();
  return params;
}

FluxAverageAux::FluxAverageAux(const InputParameters & parameters)
  : NormalDiffusionFluxAux(parameters)
{
  mooseError("The 'FluxAverageAux' auxkernel has been remaned to 'NormalDiffusionFluxAux'");
}
