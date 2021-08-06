//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KEigenvalue.h"
#include "openmc/eigenvalue.h"

registerMooseObject("CardinalApp", KEigenvalue);

defineLegacyParams(KEigenvalue);

InputParameters
KEigenvalue::validParams()
{
  InputParameters params = OpenMCPostprocessor::validParams();
  params.addParam<MooseEnum>("value_type", getEigenvalueEnum(),
    "Type of eigenvalue global tally to report; options: collision, absorption, tracklength, combined (default)");
  params.addClassDescription("Extract the k eigenvalue computed by OpenMC");
  return params;
}

KEigenvalue::KEigenvalue(const InputParameters & parameters) :
  OpenMCPostprocessor(parameters),
  _type(getParam<MooseEnum>("value_type").getEnum<eigenvalue::EigenvalueEnum>())
{
}

Real
KEigenvalue::getValue()
{
  int n = openmc::simulation::n_realizations;
  const auto & gt = openmc::simulation::global_tallies;

  switch (_type)
  {
    case eigenvalue::collision:
      return gt(openmc::GlobalTally::K_COLLISION, openmc::TallyResult::SUM) / n;
    case eigenvalue::absorption:
      return gt(openmc::GlobalTally::K_ABSORPTION, openmc::TallyResult::SUM) / n;
    case eigenvalue::tracklength:
      return gt(openmc::GlobalTally::K_TRACKLENGTH, openmc::TallyResult::SUM) / n;
    case eigenvalue::combined:
    {
      if (n <= 3)
        mooseError("Cannot compute combined k-effective estimate with fewer than 4 realizations!");

      double k_eff[2];
      openmc::openmc_get_keff(k_eff);
      return k_eff[0];
    }
    default:
      mooseError("Unhandled EigenvalueEnum in KEigenvalue!");
  }
}
