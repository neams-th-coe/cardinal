//
// Created by Ronald Rahaman on 2019-01-23.
//

#include "ExternalOpenmcProblem.h"
#include "openmc.h"


// TODO: registerMooseObject ?

template<>
InputParameters
validParams<ExternalOpenmcProblem>()
{
  InputParameters params = validParams<ExternalProblem>();
  params.addRequiredParam<Real>("power", "specified power for OpenMC");

  return params;
}

ExternalOpenmcProblem::ExternalOpenmcProblem(const InputParameters &params) : ExternalProblem(params)
{
}

void ExternalOpenmcProblem::externalSolve()
{
  _console << "Beginning OpenMC external solve";
  openmc_run();
}

void ExternalOpenmcProblem::syncSolutions(ExternalProblem::Direction direction)
{
}

