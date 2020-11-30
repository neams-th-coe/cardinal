#include "NekTimeStepper.h"
#include "MooseApp.h"
#include "Transient.h"
#include "nekrs.hpp"

registerMooseObject("NekApp", NekTimeStepper);

template<>
InputParameters validParams<NekTimeStepper>()
{
  InputParameters params = validParams<TimeStepper>();
  return params;
}

NekTimeStepper::NekTimeStepper(const InputParameters & parameters) :
    TimeStepper(parameters)
{
  // The NekTimeStepper will take the end time from the nekRS .par file,
  // unless NekProblem is a sub-app to a higher-up master app. In that case,
  // we will use whatever end_time is specified by the controlling app. We
  // can achieve this behavior here by setting the nekRS end_time to a very
  // high number, such that only the controlling app's end time has a say in
  // when the solution ends.
  MooseApp& app = getMooseApp();
  if (app.isUltimateMaster())
  {
    _num_time_steps = nekrs::NtimeSteps();

   // TODO: this can be changed to nekrs::finalTime() once Stefan approves PR #178
   // that fixes an internal nekRS inconsistency
    _end_time = nekrs::dt() * _num_time_steps;
  }
  else
  {
    // Typically, we would set end_time here to the maximum value this operating
    // system can handle, and set the number of time steps equal to this end time
    // divided by the specified time step size. However, this would cause integer
    // overflow in the number of time steps. So, instead here we set the number
    // of time steps to the maximum operating system value, and compute the end
    // time accordingly (since Real has a larger range in representable numbers).
    // This is just a placeholder anyways to ensure that the master app has control.
    _num_time_steps = std::numeric_limits<int>::max();
    _end_time = _num_time_steps * nekrs::dt();
  }

  // When using this time stepper, the user should not try to set any constantDT-type
  // time stepping parameters with the Transient executioner. Most of the other parameters in the
  // Transient-type executioner are also ignored, but here we only warn the user for the
  // time-step selection options.
  std::vector<std::string> invalid_params = {"start_time", "end_time", "dt", "dtmin", "dtmax", "num_steps"};
  for (const auto & s : invalid_params)
    if (_executioner.parameters().isParamSetByUser(s))
      mooseWarning("Parameter '" + s + "' is unused by the Executioner because it is " +
        "already specified by 'NekTimeStepper'!");
}

Real
NekTimeStepper::computeInitialDT()
{
  return nekrs::dt();
}

Real
NekTimeStepper::computeDT()
{
  return nekrs::dt();
}

Real
NekTimeStepper::getDT()
{
  return nekrs::dt();
}

Real
NekTimeStepper::getEndTime()
{
  return _end_time;
}

Real
NekTimeStepper::getNumTimeSteps()
{
  return _num_time_steps;
}
