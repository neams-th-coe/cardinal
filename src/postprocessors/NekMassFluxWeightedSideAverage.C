//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NekMassFluxWeightedSideAverage.h"

registerMooseObject("CardinalApp", NekMassFluxWeightedSideAverage);

defineLegacyParams(NekMassFluxWeightedSideAverage);

InputParameters
NekMassFluxWeightedSideAverage::validParams()
{
  InputParameters params = NekMassFluxWeightedSideIntegral::validParams();
  return params;
}

NekMassFluxWeightedSideAverage::NekMassFluxWeightedSideAverage(const InputParameters & parameters) :
  NekMassFluxWeightedSideIntegral(parameters)
{
}

Real
NekMassFluxWeightedSideAverage::getValue()
{
  return NekMassFluxWeightedSideIntegral::getValue() / nekrs::massFlowrate(_boundary);
}
