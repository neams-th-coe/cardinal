#include "NekTimeStepper.h"
#include "MooseApp.h"
#include "Transient.h"
#include "NekInterface.h"
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
  // nekRS can end a simulation based on (1) a number of time steps, (2) an
  // end time, or (3) a total elapsed wall time. Because this wall timer would
  // keep incrementing while other applications are running in this multiphysics
  // environment, we don't want to base when to finish nekRS on this. Even if
  // NekApp is the ultimate master app, we still probably don't want to use a wall
  // time to determine when to end the simulation, because other objects in the
  // MOOSE input file would consume that wall time.
  if (nekrs::endControlElapsedTime())
    mooseError("A wall time cannot be used to control the end of the nekRS simulation "
      "when other applications are consuming the same wall time.\n\nPlease set "
      "'stopAt' to either 'numSteps' or 'endTime' in your .par file.");

  // The NekTimeStepper will take the end simulation control from the nekRS .par file,
  // unless NekProblem is a sub-app to a higher-up master app. In that case,
  // we will use whatever end control is specified by the controlling app.
  MooseApp& app = getMooseApp();
  if (app.isUltimateMaster())
  {
    // The MOOSE Transient executioner will end the simulation if _either_ the number
    // of time steps is greater than Transient::_num_steps or the time is greater than
    // or equal to Transient::_end_time. To avoid conflicts with NekTimeStepper, below
    // we throw an error if the user tries to set num_steps or end_time from the NekApp
    // input file. This guarantees that the MOOSE defaults of numeric_max are kept for
    // both end_time and num_steps, such that our nekRS setting will prevail.
    if (nekrs::endControlTime())
      _end_time = nekrs::endTime();
    else if (nekrs::endControlNumSteps())
      _num_steps = nekrs::numSteps();
  }
  else
  {
    // To allow the controlling app to dictate when the nekRS solution ends, we
    // just need to have the NekApp have a very large end_time (that presumably
    // the controlling app would not try to simulate beyond). We don't need to do
    // anything here, since the mooseError below ensures that we retain the MOOSE defaults
    // of numeric_max for both end_time and num_steps.
  }

  // When using this time stepper, the user should not try to set any constantDT-type
  // time stepping parameters with the Transient executioner. These could potentially
  // interfere with what nekRS is trying to use.
  std::vector<std::string> invalid_params = {"start_time", "end_time", "dt", "dtmin", "dtmax", "num_steps"};
  for (const auto & s : invalid_params)
    if (_executioner.parameters().isParamSetByUser(s))
      mooseError("Parameter '" + s + "' is unused by the Executioner because it is " +
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
