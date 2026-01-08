/********************************************************************/
/*                  SOFTWARE COPYRIGHT NOTIFICATION                 */
/*                             Cardinal                             */
/*                                                                  */
/*                  (c) 2021 UChicago Argonne, LLC                  */
/*                        ALL RIGHTS RESERVED                       */
/*                                                                  */
/*                 Prepared by UChicago Argonne, LLC                */
/*               Under Contract No. DE-AC02-06CH11357               */
/*                With the U. S. Department of Energy               */
/*                                                                  */
/*             Prepared by Battelle Energy Alliance, LLC            */
/*               Under Contract No. DE-AC07-05ID14517               */
/*                With the U. S. Department of Energy               */
/*                                                                  */
/*                 See LICENSE for full restrictions                */
/********************************************************************/

#ifdef ENABLE_NEK_COUPLING

#include "NekTimeStepper.h"
#include "MooseApp.h"
#include "Transient.h"
#include "NekInterface.h"

registerMooseObject("CardinalApp", NekTimeStepper);

InputParameters
NekTimeStepper::validParams()
{
  InputParameters params = TimeStepper::validParams();
  params.addParam<Real>("min_dt", 1e-9, "Minimum time step size to allow MOOSE to set in nekRS");
  params.addClassDescription("Select time step size based on NekRS time stepping schemes");
  return params;
}

NekTimeStepper::NekTimeStepper(const InputParameters & parameters)
  : TimeStepper(parameters), _min_dt(getParam<Real>("min_dt"))
{
  // Set a higher value for the timestep tolerance with which time steps are
  // compared between nekRS and other MOOSE apps in a multiapp hierarchy. For some reason,
  // it seems that floating point round-off accumulation is significant in nekRS, such
  // that with subcycling, I frequently see that MOOSE wants nekRS to take a time step
  // on the order of 5e-14 in order to "catch up" to a synchronization point.
  _executioner.setTimestepTolerance(_min_dt);

  // nekRS can end a simulation based on (1) a number of time steps, (2) an
  // end time, or (3) a total elapsed wall time. Because this wall timer would
  // keep incrementing while other applications are running in this multiphysics
  // environment, we don't want to base when to finish nekRS on this. Even if
  // Nek is the ultimate master app, we still probably don't want to use a wall
  // time to determine when to end the simulation, because other objects in the
  // MOOSE input file would consume that wall time.
  if (nekrs::endControlElapsedTime())
    mooseError(
        "A wall time cannot be used to control the end of the nekRS simulation "
        "when other applications (i.e. MOOSE) are consuming the same wall time.\n\nPlease set "
        "'stopAt' to either 'numSteps' or 'endTime' in your .par file.");

  // The NekTimeStepper will take the end simulation control from the nekRS .par file.
  // If the Nek app is a sub-app, then the higher-up master app will automatically take
  // control of when the overall simulation ends.
  if (nekrs::endControlTime())
    _end_time = nekrs::endTime();

  if (nekrs::endControlNumSteps())
    forceNumSteps(nekrs::numSteps());

  // If running in JIT build mode, we don't want to do any time steps
  if (nekrs::buildOnly())
    forceNumSteps(0);

  // When using this time stepper, the user should not try to set any constantDT-type
  // time stepping parameters with the Transient executioner. These could potentially
  // interfere with what nekRS is trying to use.
  std::vector<std::string> invalid_params = {"end_time", "dt", "dtmin", "dtmax", "num_steps"};
  for (const auto & s : invalid_params)
    if (_executioner.parameters().isParamSetByUser(s))
      mooseError("Parameter '" + s + "' is unused by the Executioner because it is " +
                 "already specified by 'NekTimeStepper'!");

  // at this point, this is non-dimensional dt
  auto [dtSubStep, dt] = nekrs::dt(1);
  _nek_dt = dt;
}

Real
NekTimeStepper::computeInitialDT()
{
  return _nek_dt * nekrs::referenceTime();
}

Real
NekTimeStepper::computeDT()
{
  auto [dtSubStep, _dt] = nekrs::dt(_t_step);
  Real dt = nekrs::hasVariableDt() ? _dt : _nek_dt;
  return dt * nekrs::referenceTime();
}

Real
NekTimeStepper::minDT() const
{
  return _min_dt;
}

Real
NekTimeStepper::nondimensionalDT(const Real & dimensional_dt) const
{
  return dimensional_dt / nekrs::referenceTime();
}

#endif
