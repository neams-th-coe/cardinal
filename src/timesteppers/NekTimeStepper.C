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
#include "nekrs.hpp"

registerMooseObject("CardinalApp", NekTimeStepper);

InputParameters
NekTimeStepper::validParams()
{
  InputParameters params = TimeStepper::validParams();
  params.addParam<Real>("min_dt", 1e-6, "Minimum time step size to allow MOOSE to set in nekRS");
  params.addClassDescription("Select time step size based on NekRS time stepping schemes");
  return params;
}

NekTimeStepper::NekTimeStepper(const InputParameters & parameters)
  : TimeStepper(parameters), _min_dt(getParam<Real>("min_dt"))
{
  // Cardinal does not yet support variable dt in Nek; this is coming soon,
  // but just to make sure no one inadvertently tries it, check for it
  if (nekrs::hasVariableDt())
    mooseError("Cardinal does not yet support variable timestepping in NekRS!\n"
      "This feature is coming soon.");

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

  // The NekTimeStepper will take the end simulation control from the nekRS .par file,
  // unless NekRSProblem is a sub-app to a higher-up master app. In that case,
  // we will use whatever end control is specified by the controlling app.
  MooseApp & app = getMooseApp();
  if (app.isUltimateMaster())
  {
    // The MOOSE Transient executioner will end the simulation if _either_ the number
    // of time steps is greater than Transient::_num_steps or the time is greater than
    // or equal to Transient::_end_time. To avoid conflicts with NekTimeStepper, below
    // we throw an error if the user tries to set num_steps or end_time from the Nek
    // input file. This guarantees that the MOOSE defaults of numeric_max are kept for
    // both end_time and num_steps, such that our nekRS setting will prevail.
    if (nekrs::endControlTime())
      _end_time = nekrs::endTime();

    if (nekrs::endControlNumSteps())
      forceNumSteps(nekrs::numSteps());
  }
  else
  {
    // To allow the controlling app to dictate when the nekRS solution ends, we
    // just need to have the Nek have a very large end_time (that presumably
    // the controlling app would not try to simulate beyond). We don't need to do
    // anything here, since the mooseError below ensures that we retain the MOOSE defaults
    // of numeric_max for both end_time and num_steps.
  }

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

  // nekRS is currently limited to fixed time stepping. We would need to add a call to
  // a routine like nekrs::computeDT() which computes the time step based on the CFL condition,
  // but that function doesn't exist yet. We cannot just call nekrs::dt() in computeDT() here,
  // because the nrs->dt[0] variable that is returned by nekrs::dt() is the _same_ as that set
  // by MOOSE. This circular dependency was giving me floating point issues with synchronization
  // for some subcycling applications. So, until we have variable time stepping in nekRS, let's
  // set a fixed time step here.
  _nek_dt = nekrs::dt(0);
}

Real
NekTimeStepper::computeInitialDT()
{
  return _nek_dt;
}

Real
NekTimeStepper::computeDT()
{
  return _nek_dt;
}

Real
NekTimeStepper::minDT() const
{
  return _min_dt;
}

void
NekTimeStepper::setReferenceTime(const Real & L, const Real & U)
{
  _t_ref = L / U;
}

void
NekTimeStepper::dimensionalizeDT()
{
  _nek_dt *= _t_ref;
}

Real
NekTimeStepper::dimensionalDT(const Real & nondimensional_dt) const
{
  return nondimensional_dt * _t_ref;
}

Real
NekTimeStepper::nondimensionalDT(const Real & dimensional_dt) const
{
  return dimensional_dt / _t_ref;
}

#endif
