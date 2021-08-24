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
  params.addRequiredParam<MooseEnum>("field", getNekFieldEnum(), "Field to integrate;"
    "options: velocity_x, velocity_y, velocity_z, "
    "velocity, temperature, pressure, unity");
  params.addClassDescription("Compute the integral of a field over a boundary of the NekRS mesh");
  return params;
}

NekSideIntegral::NekSideIntegral(const InputParameters & parameters) :
  NekSidePostprocessor(parameters),
  _field(getParam<MooseEnum>("field").getEnum<field::NekFieldEnum>())
{
  checkValidField(_field);
}

Real
NekSideIntegral::getValue()
{
  return nekrs::sideIntegral(_boundary, _field);
}
