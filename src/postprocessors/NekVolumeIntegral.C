//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NekVolumeIntegral.h"

registerMooseObject("CardinalApp", NekVolumeIntegral);

defineLegacyParams(NekVolumeIntegral);

InputParameters
NekVolumeIntegral::validParams()
{
  InputParameters params = NekPostprocessor::validParams();
  params.addRequiredParam<MooseEnum>("field", getNekFieldEnum(), "Field to integrate; "
    "options: velocity_x, velocity_y, velocity_z, "
    "velocity, temperature, pressure, unity");
  params.addClassDescription("Compute the integral of a field over the NekRS mesh");
  return params;
}

NekVolumeIntegral::NekVolumeIntegral(const InputParameters & parameters) :
  NekPostprocessor(parameters),
  _field(getParam<MooseEnum>("field").getEnum<field::NekFieldEnum>())
{
}

Real
NekVolumeIntegral::getValue()
{
  return nekrs::volumeIntegral(_field);
}
