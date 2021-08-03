//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NekSideAverage.h"

registerMooseObject("CardinalApp", NekSideAverage);

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
  if (_fixed_mesh)
    _area = nekrs::area(_boundary);
}

Real
NekSideAverage::getValue()
{
  Real area = _fixed_mesh ? _area : nekrs::area(_boundary);
  return NekSideIntegral::getValue() / area;
}
