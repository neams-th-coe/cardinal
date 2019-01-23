//
// Created by Ronald Rahaman on 2019-01-23.
//

#include "ExternalOpenmcProblem.hpp"
#include "openmc/capi.h"


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
  _console << "Beginning OpenMC external solve";
  openmc_run();
}

