//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NekSideIntegral.h"

registerMooseObject("CardinalApp", NekSideIntegral);

defineLegacyParams(NekSideIntegral);

InputParameters
NekSideIntegral::validParams()
{
  InputParameters params = NekSidePostprocessor::validParams();
  params.addRequiredParam<MooseEnum>("field", getNekFieldEnum(), "Field to integrate");
  return params;
}

NekSideIntegral::NekSideIntegral(const InputParameters & parameters) :
  NekSidePostprocessor(parameters),
  _field(getParam<MooseEnum>("field").getEnum<field::NekFieldEnum>())
{
}

Real
NekSideIntegral::getValue()
{
  return nekrs::sideIntegral(_boundary, _field);
}
