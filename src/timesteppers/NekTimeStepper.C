#include "NekTimeStepper.h"
#include "NekInterface.h"
#include "nekrs.hpp"

template<>
InputParameters validParams<NekTimeStepper>()
{
  InputParameters params = validParams<TimeStepper>();
  params.addParam<Real>("dt", 1.0, "Size of the time step");
  return params;
}

NekTimeStepper::NekTimeStepper(const InputParameters & parameters) :
    TimeStepper(parameters),
    _dt(nekrs::dt()),
    _outputStep(nekrs::outputStep()),
    _nTimeSteps(nekrs::NtimeSteps()),
    _startTime(nekrs::startTime()),
    _finalTime(nekrs::finalTime()),
    _time(nekrs::startTime())
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
  // TODO:  Was this changed in the driver?
  if (_time < _finalTime) {

    ++_tstep;
    
    int isOutputStep = 0;
    if (_outputStep > 0) {
      if (_tstep % _outputStep == 0 || _tstep == _nTimeSteps) isOutputStep = 1;
    }

    nekrs::runStep(_time, _dt, _nTimeSteps);

    if (isOutputStep) nekrs::copyToNek(_time+_dt, _tstep);
    nekrs::udfExecuteStep(_time+_dt, _tstep, isOutputStep);
    if (isOutputStep) nekrs::nekOutfld();

    _time += _dt;

  }
}

bool
NekTimeStepper::converged()
{
  // TODO: Should report information from Nek
  return true;
}
