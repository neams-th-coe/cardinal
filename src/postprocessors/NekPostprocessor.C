//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NekPostprocessor.h"

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
  _nek_problem = dynamic_cast<const NekRSProblemBase *>(&_fe_problem);
  if (!_nek_problem)
  {
    std::string extra_help = _fe_problem.type() == "FEProblem" ? " (the default)" : "";
    mooseError("Postprocessor with name '" + name() + "' can only be used with wrapped Nek cases!\n"
      "You need to change the problem type from '" + _fe_problem.type() + "'" + extra_help +" to a Nek-wrapped problem.\n\n"
      "options: 'NekRSProblem', 'NekRSStandaloneProblem'");
  }

  _fixed_mesh = !(_nek_problem->movingMesh());

  // NekRSProblem enforces that we then use NekRSMesh, so we don't need to check that
  // this pointer isn't NULL
  _nek_mesh = dynamic_cast<const NekRSMesh *>(&_mesh);
}

void
NekPostprocessor::checkValidField(const field::NekFieldEnum & field) const
{
  if (!nekrs::hasTemperatureVariable() && field == field::temperature)
    mooseError("Postprocessor with name '" + name() + "' cannot set 'field = temperature' "
      "because your Nek case files do not have a temperature variable!");
}
