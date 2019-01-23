//
// Created by Ronald Rahaman on 2019-01-23.
//

#include "ExternalNekProblem.h"
#include "Moose.h"
#include "openmc.h"

template<>
InputParameters
validParams<ExternalNekProblem>()
{
  InputParameters params = validParams<ExternalProblem>();
  // No required parameters
  return params;
}

ExternalNekProblem::ExternalNekProblem(const InputParameters &params) : ExternalProblem(params)
{
  _console << "Beginning Nek5000 external solve";
  FORTRAN_CALL(nek_step)();
}

