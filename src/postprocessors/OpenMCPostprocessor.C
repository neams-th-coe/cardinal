//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "OpenMCPostprocessor.h"

defineLegacyParams(OpenMCPostprocessor);

InputParameters
OpenMCPostprocessor::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  return params;
}

OpenMCPostprocessor::OpenMCPostprocessor(const InputParameters & parameters) :
  GeneralPostprocessor(parameters)
{
  _openmc_problem = dynamic_cast<OpenMCCellAverageProblem *>(&_fe_problem);

  if (!_openmc_problem)
    mooseError("Postprocessor with name '" + name() + "' can only be used with OpenMCCellAverageProblem!");
}
