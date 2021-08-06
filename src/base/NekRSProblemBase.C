#include "NekRSProblemBase.h"
#include "Moose.h"
#include "AuxiliarySystem.h"
#include "TimeStepper.h"
#include "NekInterface.h"
#include "TimedPrint.h"

#include "nekrs.hpp"
#include "nekInterface/nekInterfaceAdapter.hpp"

template<>
InputParameters
validParams<NekRSProblemBase>()
{
  InputParameters params = validParams<ExternalProblem>();
  params.addParam<std::string>("casename", "Case name for the NekRS input files; "
    "this is <case> in <case>.par, <case>.udf, <case>.oudf, and <case>.re2. "
    "Can also be provided on the command line with --nekrs-setup, which will override this setting");

  params.addParam<bool>("nondimensional", false, "Whether NekRS is solved in non-dimensional form");
  params.addRangeCheckedParam<Real>("U_ref", 1.0, "U_ref > 0.0", "Reference velocity value for non-dimensional solution");
  params.addRangeCheckedParam<Real>("T_ref", 0.0, "T_ref >= 0.0", "Reference temperature value for non-dimensional solution");
  params.addRangeCheckedParam<Real>("dT_ref", 1.0, "dT_ref > 0.0", "Reference temperature range value for non-dimensional solution");
  params.addRangeCheckedParam<Real>("L_ref", 1.0, "L_ref > 0.0", "Reference length scale value for non-dimensional solution");
  params.addRangeCheckedParam<Real>("rho_0", 1.0, "rho_0 > 0.0", "Density parameter value for non-dimensional solution");
  params.addRangeCheckedParam<Real>("Cp_0", 1.0, "Cp_0 > 0.0", "Heat capacity parameter value for non-dimensional solution");
  return params;
}

NekRSProblemBase::NekRSProblemBase(const InputParameters &params) : ExternalProblem(params),
  _nondimensional(getParam<bool>("nondimensional")),
  _U_ref(getParam<Real>("U_ref")),
  _T_ref(getParam<Real>("T_ref")),
  _dT_ref(getParam<Real>("dT_ref")),
  _L_ref(getParam<Real>("L_ref")),
  _rho_0(getParam<Real>("rho_0")),
  _Cp_0(getParam<Real>("Cp_0")),
  _start_time(nekrs::startTime())
{
  _nek_mesh = dynamic_cast<NekRSMesh*>(&mesh());

  if (!_nek_mesh)
    mooseError("Mesh for '" + type() + "' must be of type 'NekRSMesh', but you have specified a '" + mesh().type() + "'!");

  // the Problem constructor is called right after building the mesh. In order
  // to have pretty screen output without conflicting with the timed print messages,
  // print diagnostic info related to the mesh here
  _nek_mesh->printMeshInfo();

  // if solving in nondimensional form, make sure that the user specified _all_ of the
  // necessary scaling quantities to prevent errors from forgetting one, which would take
  // a non-scaled default otherwise
  std::vector<std::string> scales = {"U_ref", "T_ref", "dT_ref", "L_ref", "rho_0", "Cp_0"};
  std::vector<std::string> descriptions = {"velocity", "temperature", "temperature range", "length",
    "density", "heat capacity"};
  for (std::size_t n = 0; n < scales.size(); ++n)
  {
    if (_nondimensional && !params.isParamSetByUser(scales[n]))
      paramError(scales[n], "When NekRS solves in non-dimensional form, a characterstic " + descriptions[n] + " must be provided!");
    else if (!_nondimensional && params.isParamSetByUser(scales[n]))
      mooseWarning("When NekRS solves in dimensional form, " + descriptions[n] + " is unused!");
  }

  // inform NekRS of the scaling that we are using if solving in non-dimensional form
  nekrs::solution::initializeDimensionalScales(_U_ref, _T_ref, _dT_ref, _L_ref, _rho_0, _Cp_0);

  if (_nondimensional)
    _console << "The NekRS model uses the following non-dimensional scales: " <<
      "\n length: " << _L_ref << "\n velocity: " << _U_ref << "\n temperature: " << _T_ref <<
      "\n temperature increment: " << _dT_ref << std::endl;
}

NekRSProblemBase::~NekRSProblemBase()
{
}

void
NekRSProblemBase::initialSetup()
{
  ExternalProblem::initialSetup();

  auto executioner = _app.getExecutioner();
  _transient_executioner = dynamic_cast<Transient *>(executioner);

  // NekRS only supports transient simulations - therefore, it does not make
  // sense to use anything except a Transient-derived executioner
  if (!_transient_executioner)
    mooseError("A 'Transient' executioner must be used with '" + type() + "', but "
      "you have specified the '" + executioner->type() + "' executioner!");

  // If the simulation start time is not zero, the app's time must be shifted
  // relative to its master app (if any). Until this is implemented, make sure
  // a start time of zero is used.
  const auto moose_start_time = _transient_executioner->getStartTime();
  if (moose_start_time != 0.0)
    mooseError("You specified a start time of " + Moose::stringify(moose_start_time) +
      ", but a non-zero start time is not yet supported for '" + type() + "'!");

  // To get the correct time stepping information on the MOOSE side, we also
  // must use the NekTimeStepper
  TimeStepper * stepper = _transient_executioner->getTimeStepper();
  _timestepper = dynamic_cast<NekTimeStepper *>(stepper);
  if (!_timestepper)
    mooseError("The 'NekTimeStepper' stepper must be used with '" + type() + "', "
      "but you have specified the '" + stepper->type() + "' time stepper!");

  // Set the reference time for use in dimensionalizing/non-dimensionalizing the time
  _timestepper->setReferenceTime(_L_ref, _U_ref);

  // Also make sure that the start time is consistent with what MOOSE wants to use.
  // If different from what NekRS internally wants to use, use the MOOSE value.
  if (!MooseUtils::absoluteFuzzyEqual(moose_start_time, _start_time))
  {
    mooseWarning("The start time set on the executioner: " + Moose::stringify(moose_start_time) +
      " does not match the start time set in NekRS's .par file: " + Moose::stringify(_timestepper->dimensionalDT(_start_time)) + ". "
      "This may happen if you are using a restart file in NekRS.\n\n" +
      "Setting start time for '" + type() + "' to: " + Moose::stringify(moose_start_time));
    _start_time = moose_start_time;
  }

  // Then, dimensionalize the NekRS time so that all occurrences of _dt here are
  // in dimensional form
  _timestepper->dimensionalizeDT();

  // nekRS calls UDF_ExecuteStep once before the time stepping begins
  nekrs::udfExecuteStep(_start_time, _t_step, false /* not an output step */);
}

void NekRSProblemBase::externalSolve()
{
  // _dt reflects the time step that MOOSE wants Nek to
  // take. For instance, if Nek is controlled by a master app and subcycling is used,
  // Nek must advance to the time interval taken by the master app. If the time step
  // that MOOSE wants nekRS to take (i.e. _dt) is smaller than we'd like nekRS to take, error.
  if (_dt < _timestepper->minDT())
    mooseError("Requested time step of " + std::to_string(_dt) + " is smaller than the minimum "
      "time step of " + Moose::stringify(_timestepper->minDT()) + " allowed in NekRS!\n\n"
      "You can control this behavior with the 'min_dt' parameter on 'NekTimeStepper'.");

  // _time represents the time that we're simulating _to_, but we need to pass sometimes slightly different
  // times into the nekRS routines, which assume that the "time" passed into their
  // routines is sometimes a different interpretation.
  double step_start_time = _time - _dt;
  double step_end_time = _time;

  // Run a nekRS time step. After the time step, this also calls UDF_ExecuteStep,
  // evaluated at (step_end_time, _t_step)
  nekrs::runStep(_timestepper->nondimensionalDT(step_start_time),
    _timestepper->nondimensionalDT(_dt), _t_step);

  // optional entry point to adjust the recently-computed NekRS solution
  adjustNekSolution();

  // Note: here, we copy to both the nrs solution arrays and to the Nek5000 backend arrays,
  // because it is possible that users may interact using the legacy usr-file approach.
  // If we move away from the Nek5000 backend entirely, we could replace this line with
  // direct OCCA memcpy calls. But we do definitely need some type of copy here for _every_
  // time step, even if we're not technically passing data to another app, because we have
  // postprocessors that touch the `nrs` arrays that can be called in an arbitrary fashion
  // by the user.
  nek::ocopyToNek(_timestepper->nondimensionalDT(step_end_time), _t_step);

  if (isOutputStep())
    nekrs::outfld(_timestepper->nondimensionalDT(step_end_time));

  _time += _dt;
}

bool
NekRSProblemBase::isOutputStep() const
{
  if (_app.isUltimateMaster())
  {
    bool last_step = nekrs::lastStep(_timestepper->nondimensionalDT(_time), _t_step, 0.0 /* dummy elapsed time */);

    // if Nek is controlled by a master application, then the last time step
    // is controlled by that master application, in which case we don't want to
    // write at what nekRS thinks is the last step (since it may or may not be
    // the actual end step), especially because we already ensure that we write on the
    // last time step from MOOSE's perspective in NekRSProblem's destructor.
    if (last_step)
      return true;
  }

  // this routine does not check if we are on the last step - just whether we have
  // met the requested runtime or time step interval
  return nekrs::outputStep(_timestepper->nondimensionalDT(_time), _t_step);
}
