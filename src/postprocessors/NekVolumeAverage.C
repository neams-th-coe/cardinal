//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NekVolumeAverage.h"

registerMooseObject("CardinalApp", NekVolumeAverage);

defineLegacyParams(NekVolumeAverage);

InputParameters
NekVolumeAverage::validParams()
{
  InputParameters params = NekVolumeIntegral::validParams();
  return params;
}

NekVolumeAverage::NekVolumeAverage(const InputParameters & parameters) :
  NekVolumeIntegral(parameters)
{
  if (_fixed_mesh)
    _volume = nekrs::volume();
}

Real
NekVolumeAverage::getValue()
{
  Real volume = _fixed_mesh ? _volume : nekrs::volume();
  return NekVolumeIntegral::getValue() / volume;
}
