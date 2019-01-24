//
// Created by Ronald Rahaman on 2019-01-23.
//

#include "NekProblem.h"
#include "Moose.h"
#include "NekInterface.h"

registerMooseObject("MooseApp", NekProblem);

template<>
InputParameters
validParams<NekProblem>()
{
  InputParameters params = validParams<ExternalProblem>();
  // No required parameters
  return params;
}

NekProblem::NekProblem(const InputParameters &params) : ExternalProblem(params)
{
}


void NekProblem::externalSolve()
{
  _console << "Beginning Nek5000 external solve";
  Nek5000::FORTRAN_CALL(nek_init_step)();
  Nek5000::FORTRAN_CALL(nek_step)();
  Nek5000::FORTRAN_CALL(nek_finalize_step)();
}

void NekProblem::syncSolutions(ExternalProblem::Direction direction)
{

}
