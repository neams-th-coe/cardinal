//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NekVolumeAverage.h"

registerMooseObject("NekApp", NekVolumeAverage);

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
  // it's probably a mistake to supply unity to average, since you'd
  // always just compute 1
  if (_field == field::unity)
    mooseWarning("Providing a 'field' of 'unity' to 'NekVolumeAverage' will compute "
      "a volume-average of the volume, which will always be unity.");
}

Real
NekVolumeAverage::getValue()
{
  // normalize by the side integral of unity, i.e. the area integral
  const field::NekFieldEnum unity = field::unity;
  return NekVolumeIntegral::getValue() / nekrs::volumeIntegral(unity);
}
