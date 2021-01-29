//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NekSideAverage.h"

registerMooseObject("NekApp", NekSideAverage);

defineLegacyParams(NekSideAverage);

InputParameters
NekSideAverage::validParams()
{
  InputParameters params = NekSideIntegral::validParams();
  return params;
}

NekSideAverage::NekSideAverage(const InputParameters & parameters) :
  NekSideIntegral(parameters)
{
  // it's probably a mistake to supply unity to average, since you'd
  // always just compute 1
  if (_field == field::unity)
    mooseWarning("Providing a 'field' of 'unity' to 'NekSideAverage' will compute "
      "an area-average of the area, which will always be unity.");
}

Real
NekSideAverage::getValue()
{
  // normalize by the side integral of unity, i.e. the area integral
  const field::NekFieldEnum unity = field::unity;
  return NekSideIntegral::getValue() / nekrs::sideIntegral(_boundary, unity);
}
