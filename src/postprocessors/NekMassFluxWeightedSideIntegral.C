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
  InputParameters params = NekSideIntegral::validParams();
  params.addClassDescription("Compute mass flux weighted integral of a field over a boundary of the NekRS mesh");
  return params;
}

NekMassFluxWeightedSideIntegral::NekMassFluxWeightedSideIntegral(const InputParameters & parameters) :
  NekSideIntegral(parameters)
{
}

Real
NekMassFluxWeightedSideIntegral::getValue()
{
  return nekrs::sideMassFluxWeightedIntegral(_boundary, _field);
}
