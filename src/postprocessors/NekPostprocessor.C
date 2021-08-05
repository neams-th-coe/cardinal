//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NekPostprocessor.h"
#include "NekRSProblem.h"

defineLegacyParams(NekPostprocessor);

InputParameters
NekPostprocessor::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  return params;
}

NekPostprocessor::NekPostprocessor(const InputParameters & parameters) :
  GeneralPostprocessor(parameters),
  _mesh(_subproblem.mesh())
{
  _nek_problem = dynamic_cast<const NekRSProblem *>(&_fe_problem);
  if (!_nek_problem)
    mooseError("Postprocessor with name '" + name() + "' can only be used with NekRSProblem!");

  _fixed_mesh = !(_nek_problem->movingMesh());

  // NekRSProblem enforces that we then use NekRSMesh, so we don't need to check that
  // this pointer isn't NULL
  _nek_mesh = dynamic_cast<const NekRSMesh *>(&_mesh);
}
