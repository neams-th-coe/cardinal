//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NekMassFluxWeightedSideAverage.h"

registerMooseObject("NekApp", NekMassFluxWeightedSideAverage);

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
  // normalize by mass flow rate, i.e. mass flux weighted integral of unity
  const field::NekFieldEnum unity = field::unity;
  return NekMassFluxWeightedSideIntegral::getValue() / nekrs::sideMassFluxWeightedIntegral(_boundary, unity);
}
