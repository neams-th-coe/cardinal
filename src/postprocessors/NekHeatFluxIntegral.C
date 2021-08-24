//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NekHeatFluxIntegral.h"
#include "CardinalEnums.h"

registerMooseObject("CardinalApp", NekHeatFluxIntegral);

defineLegacyParams(NekHeatFluxIntegral);

InputParameters
NekHeatFluxIntegral::validParams()
{
  InputParameters params = NekSidePostprocessor::validParams();
  params.addClassDescription("Compute heat flux over a boundary in the NekRS mesh");
  return params;
}

NekHeatFluxIntegral::NekHeatFluxIntegral(const InputParameters & parameters) :
  NekSidePostprocessor(parameters)
{
  // this postprocessor computes the gradient of temperature, so it requires
  // the temperature field to exist
  checkValidField(field::temperature);
}

Real
NekHeatFluxIntegral::getValue()
{
  return nekrs::heatFluxIntegral(_boundary);
}
