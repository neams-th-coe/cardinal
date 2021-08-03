//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NekMassFluxWeightedSideIntegral.h"

registerMooseObject("CardinalApp", NekMassFluxWeightedSideIntegral);

defineLegacyParams(NekMassFluxWeightedSideIntegral);

InputParameters
NekMassFluxWeightedSideIntegral::validParams()
{
  InputParameters params = NekSidePostprocessor::validParams();
  params.addRequiredParam<MooseEnum>("field", getNekFieldEnum(), "Field, multiplied by mass flux, to integrate");
  return params;
}

NekMassFluxWeightedSideIntegral::NekMassFluxWeightedSideIntegral(const InputParameters & parameters) :
  NekSidePostprocessor(parameters),
  _field(getParam<MooseEnum>("field").getEnum<field::NekFieldEnum>())
{
}

Real
NekMassFluxWeightedSideIntegral::getValue()
{
  return nekrs::sideMassFluxWeightedIntegral(_boundary, _field);
}
