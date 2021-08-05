//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ReynoldsNumber.h"
#include "NekInterface.h"

registerMooseObject("CardinalApp", ReynoldsNumber);

defineLegacyParams(ReynoldsNumber);

InputParameters
ReynoldsNumber::validParams()
{
  InputParameters params = NekSidePostprocessor::validParams();
  params.addRangeCheckedParam<Real>("L_ref", "L_ref > 0.0", "Reference length scale");
  params.addClassDescription("Compute the Reynolds number characteristic of the NekRS solution");
  return params;
}

ReynoldsNumber::ReynoldsNumber(const InputParameters & parameters) :
  NekSidePostprocessor(parameters)
{
  // for dimensional cases, we need to provide the characteristic length ourselves
  // because there's no way to infer it
  if (!_nek_problem->nondimensional())
  {
    if (!isParamValid("L_ref"))
      paramError("L_ref", "When running NekRS in dimensional form, you must provide the "
        "characteristic length manually!");

    _L_ref = &getParam<Real>("L_ref");
  }

  if (_nek_problem->nondimensional() && isParamValid("L_ref"))
    mooseWarning("When NekRS solves in non-dimensional form, 'L_ref' is unused by 'ReynoldsNumber' "
      "because the setting can be inferred from NekRSProblem");

  if (_fixed_mesh)
    _area = nekrs::area(_boundary);
}

Real
ReynoldsNumber::getValue()
{
  Real area = _fixed_mesh ? _area : nekrs::area(_boundary);
  Real mdot = std::abs(nekrs::sideMassFluxWeightedIntegral(_boundary, field::unity));
  Real mu = nekrs::viscosity();
  Real L  = _nek_problem->nondimensional() ? nekrs::characteristicLength() : *_L_ref;

  return mdot * L / (area * mu);
}
