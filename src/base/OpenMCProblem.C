//
// Created by Ronald Rahaman on 2019-01-23.
//

#include "OpenMCProblem.h"
#include "openmc.h"


// TODO: registerMooseObject ?

template<>
InputParameters
validParams<OpenMCProblem>()
{
  InputParameters params = validParams<ExternalProblem>();
  params.addRequiredParam<Real>("power", "specified power for OpenMC");

  return params;
}

OpenMCProblem::OpenMCProblem(const InputParameters &params) : ExternalProblem(params)
{
}

void OpenMCProblem::externalSolve()
{
  _console << "Beginning OpenMC external solve";
  openmc_run();
}

void OpenMCProblem::syncSolutions(ExternalProblem::Direction direction)
{
}
