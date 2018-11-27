#include "NekTimeStepper.h"
#include "NekInterface.h"

template<>
InputParameters validParams<NekTimeStepper>()
{
  InputParameters params = validParams<TimeStepper>();
  params.addParam<Real>("dt", 1.0, "Size of the time step");
  return params;
}

NekTimeStepper::NekTimeStepper(const InputParameters & parameters) :
    TimeStepper(parameters),
    _dt(getParam<Real>("dt"))
{
}

Real
NekTimeStepper::computeInitialDT()
{
  // TODO: Should report information from Nek
  return _dt;
}

Real
NekTimeStepper::computeDT()
{
  // TODO: Should report information from Nek
  return _dt;
}

void
NekTimeStepper::step()
{
  FORTRAN_CALL(Nek5000::nek_step)();
}

bool
NekTimeStepper::converged()
{
  // TODO: Should report information from Nek
  return true;
}
