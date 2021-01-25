#include "NekRSProblem.h"
#include "Moose.h"
#include "AuxiliarySystem.h"
#include "TimeStepper.h"
#include "NekInterface.h"

#include "nekrs.hpp"
#include "nekInterface/nekInterfaceAdapter.hpp"

registerMooseObject("NekApp", NekRSProblem);

template<>
InputParameters
validParams<NekRSProblem>()
{
  InputParameters params = validParams<ExternalProblem>();
  params.addParam<bool>("minimize_transfers_in", false, "Whether to only synchronize nekRS "
    "for the direction TO_EXTERNAL_APP on multiapp synchronization steps");
  params.addParam<PostprocessorName>("transfer_in", "Postprocessor providing an indication "
    "of when a synchronization step has just occurred. The 'execute_on' setting for this "
    "transfer MUST be the same as the 'to_app' transfer into nekRS.");
  params.addParam<bool>("minimize_transfers_out", false, "Whether to only synchronize nekRS "
    "for the direction FROM_EXTERNAL_APP on multiapp synchronization steps");
  return params;
}

NekRSProblem::NekRSProblem(const InputParameters &params) : ExternalProblem(params),
    _serialized_solution(NumericVector<Number>::build(_communicator).release()),
    _minimize_transfers_in(getParam<bool>("minimize_transfers_in")),
    _minimize_transfers_out(getParam<bool>("minimize_transfers_out")),
    _start_time(nekrs::startTime())
{
  if (_minimize_transfers_in && !isParamValid("transfer_in"))
    mooseError("Setting 'minimize_transers_in' to true requires the 'transfer_in' postprocessor!");

  if (!_minimize_transfers_in && isParamValid("transfer_in"))
    mooseWarning("'transfer_in' is unused when 'minimize_transfers_in' is set to false!");

  _nek_mesh = dynamic_cast<NekRSMesh*>(&mesh());

  if (!_nek_mesh)
    mooseError("Mesh for a 'NekRSProblem' must be of type 'NekRSMesh'! In your [Mesh] "
      "block, you should have 'type = NekRSMesh'");

  _n_surface_elems = _nek_mesh->numSurfaceElems();
  _n_vertices_per_face = _nek_mesh->numVerticesPerFace();
  _boundary = _nek_mesh->boundary();

  _T = (double*) calloc(_n_surface_elems * _n_vertices_per_face, sizeof(double));
  _flux_face = (double *) calloc(_n_vertices_per_face, sizeof(double));

  // determine the interpolation matrix for the temperature transfer
  int starting_points = _nek_mesh->nekNumQuadraturePoints1D();
  int ending_points = _nek_mesh->numQuadraturePoints1D();
  _temp_interpolation_matrix = (double *) calloc(starting_points * ending_points, sizeof(double));
  nekrs::interpolationMatrix(_temp_interpolation_matrix, starting_points, ending_points);

  // determine the interpolation matrix for the flux transfer
  std::swap(starting_points, ending_points);
  _flux_interpolation_matrix = (double *) calloc(starting_points * ending_points, sizeof(double));
  nekrs::interpolationMatrix(_flux_interpolation_matrix, starting_points, ending_points);

  // we can save some effort for the low-order situations where the interpolation
  // matrix is the identity matrix (i.e. for which equi-spaced libMesh nodes are an
  // exact subset of the nekRS GLL points). This will happen for any first-order mesh,
  // and if a second-order mesh is used with a polynomial order of 2 in nekRS. Because
  // we pretty much always use a polynomial order greater than 2 in nekRS, let's just
  // check the first case because this will simplify our code in the nekrs::temperature()
  // function. If you change this line, you MUST change the innermost if/else statement
  // in nekrs::temperature!
  _needs_interpolation = _nek_mesh->numQuadraturePoints1D() > 2;
}

NekRSProblem::~NekRSProblem()
{
  // write nekRS solution to output if not already written for this step
  if (!isOutputStep())
    nekrs::outfld(_time);

  if (_temp_interpolation_matrix) free(_temp_interpolation_matrix);
  if (_flux_interpolation_matrix) free(_flux_interpolation_matrix);
  if (_T) free(_T);
  if (_flux_face) free(_flux_face);
}

void
NekRSProblem::initialSetup()
{
  ExternalProblem::initialSetup();

  auto executioner = _app.getExecutioner();
  _transient_executioner = dynamic_cast<Transient *>(executioner);

  // nekRS only supports transient simulations - therefore, it does not make
  // sense to use anything except a Transient-derived executioner
  if (!_transient_executioner)
    mooseError("A Transient-type executioner should be used for nekRS!");

  // If the simulation start time is not zero, the app's time must be shifted
  // relative to its master app (if any). Until this is implemented, make sure
  // a start time of zero is used.
  const auto moose_start_time = _transient_executioner->getStartTime();
  if (moose_start_time != 0.0)
    mooseError("A non-zero start time is not yet available for 'NekRSProblem'!");

  // Also make sure that the start time is consistent with what MOOSE wants to use.
  // If different from what nekRS internally wants to use, use the MOOSE value.
  if (std::abs(moose_start_time - _start_time) > 1e-8)
  {
    mooseWarning("The start time set on 'NekRSProblem': " + Moose::stringify(moose_start_time) +
      " does not match the start time set in nekRS's .par file: " + Moose::stringify(_start_time) + ". "
      "This may happen if you are using a restart file in nekRS.\n\n" +
      "Setting start time for 'NekRSProblem' to: " + Moose::stringify(moose_start_time));
    _start_time = moose_start_time;
  }

  // To get the correct time stepping information on the MOOSE side, we also
  // must use the NekTimeStepper
  TimeStepper * stepper = _transient_executioner->getTimeStepper();
  _timestepper = dynamic_cast<NekTimeStepper *>(stepper);
  if (!_timestepper)
    mooseError("The 'NekTimeStepper' stepper must be used with 'NekRSProblem'!");

  _flux_integral = &getPostprocessorValueByName("flux_integral");

  if (_minimize_transfers_in)
  {
    _transfer_in_name = &getParam<PostprocessorName>("transfer_in");
    _transfer_in = &getPostprocessorValueByName(*_transfer_in_name);
  }
}

bool
NekRSProblem::isOutputStep() const
{
  if (_app.isUltimateMaster())
  {
    bool last_step = nekrs::lastStep(_time, _t_step, 0.0 /* dummy elapsed time */);

    // if NekApp is controlled by a master application, then the last time step
    // is controlled by that master application, in which case we don't want to
    // write at what nekRS thinks is the last step (since it may or may not be
    // the actual end step), especially because we already ensure that we write on the
    // last time step from MOOSE's perspective in NekRSProblem's destructor.
    if (last_step)
      return true;
  }

  // this routine does not check if we are on the last step - just whether we have
  // met the requested runtime or time step interval
  return nekrs::isOutputStep(_time, _t_step);
}

void NekRSProblem::externalSolve()
{
  // The _dt member of NekRSProblem reflects the time step that MOOSE wants NekApp to
  // take. For instance, if NekApp is controlled by a master app and subcycling is used,
  // NekApp must advance to the time interval taken by the master app. If the time step
  // that MOOSE wants nekRS to take (i.e. _dt) is smaller than we'd like nekRS to take, error.
  if (_dt < _timestepper->minDT())
    mooseError("Requested time step of " + std::to_string(_dt) + " is smaller than the minimum "
      "time step allowed in nekRS!");

  // By using the _time object on the ExternalProblem base class (which represents the
  // time that we're simulating _to_, we need to pass sometimes slightly different
  // times into the nekRS routines, which assume that the "time" passed into their
  // routines is sometimes a different interpretation.
  double step_start_time = _time - _dt;
  double step_end_time = _time;

  bool is_output_step = isOutputStep();

  nekrs::runStep(step_start_time, _dt, _t_step);

  // Note: here, we copy to both the nrs solution arrays and to the Nek5000 backend arrays,
  // because it is possible that users may interact using the legacy usr-file approach.
  // If we move away from the Nek5000 backend entirely, we could replace this line with
  // direct OCCA memcpy calls. But we do definitely need some type of copy here for _every_
  // time step, even if we're not technically passing data to another app, because we have
  // postprocessors that touch the `nrs` arrays that can be called in an arbitrary fashion
  // by the user.
  nekrs::copyToNek(step_end_time, _t_step);

  nekrs::udfExecuteStep(step_end_time, _t_step, is_output_step);

  if (is_output_step)
    nekrs::outfld(step_end_time);

  _time += _dt;
}

bool
NekRSProblem::synchronizeIn()
{
  bool synchronize = true;

  if (_minimize_transfers_in)
  {
    if (*_transfer_in == false)
      synchronize = false;
    else
      setPostprocessorValueByName(*_transfer_in_name, false, 0);
  }

  return synchronize;
}

bool
NekRSProblem::synchronizeOut()
{
  bool synchronize = true;

  if (_minimize_transfers_out)
  {
    if (std::abs(_time - _dt - _transient_executioner->getTargetTime()) > _transient_executioner->timestepTol())
      synchronize = false;
  }

  return synchronize;
}

void NekRSProblem::syncSolutions(ExternalProblem::Direction direction)
{
  switch(direction)
  {
    case ExternalProblem::Direction::TO_EXTERNAL_APP:
    {
      if (!synchronizeIn())
      {
        _console << "Skipping heat flux transfer to nekRS, not at synchronization step" << std::endl;
        return;
      }

      _console << "Sending heat flux to nekRS boundary " << Moose::stringify(_boundary) << "... " << std::endl;

      auto & mesh = _nek_mesh->getMesh();
      auto & solution = _aux->solution();
      auto sys_number = _aux->number();

      static bool first = true;

      if (first)
      {
        _serialized_solution->init(_aux->sys().n_dofs(), false, SERIAL);
        first = false;
      }

      solution.localize(*_serialized_solution);

      for (unsigned int e = 0; e < _n_surface_elems; e++)
      {
        auto elem_ptr = mesh.elem_ptr(e);

        for (unsigned int n = 0; n < _n_vertices_per_face; n++)
        {
          auto node_ptr = elem_ptr->node_ptr(n);

          // For each face, get the flux at the libMesh nodes. This will be passed into
          // nekRS, which will interpolate onto its GLL points. Because we are looping over
          // nodes from libMesh, we need to get the GLL index known by nekRS and use it to
          // determine the offset in the nekRS arrays.
          int node_index = _nek_mesh->NodeIndex(n);
          auto node_offset = e * _n_vertices_per_face + node_index;

          auto dof_idx = node_ptr->dof_number(sys_number, _avg_flux_var, 0);
          _flux_face[node_index] = (*_serialized_solution)(dof_idx);
        }

        // Now that we have the flux at the nodes of the NekRSMesh, we can interpolate them
        // onto the nekRS GLL points and then write into the flux scratch space.
        nekrs::flux(_flux_interpolation_matrix, e, _nek_mesh->order(), _flux_face);
      }

      // Because the NekMesh may be quite different from that used in the app solving for
      // the heat flux, we will need to normalize the total flux on the nekRS side by the
      // total flux computed by the coupled MOOSE app.
      const double nek_flux = nekrs::fluxIntegral();

      _console << "Normalizing total nekRS flux of " << nek_flux << " to the conserved MOOSE "
        "value of " << *_flux_integral << "..." << std::endl;

      nekrs::normalizeFlux(*_flux_integral, nek_flux);

      // We can do an extra check here to make sure that the normalization was done correctly.
      // This will check if there was perhaps a disconnect in the boundaries nekRS thinks are
      // exchanging data vs. what is actually being sent.
      const double normalized_nek_flux = nekrs::fluxIntegral();
      bool high_rel_err = std::abs(normalized_nek_flux - *_flux_integral) / *_flux_integral > 1e-6;
      bool high_abs_err = std::abs(normalized_nek_flux - *_flux_integral) > 1e-6;

      if (high_rel_err || high_abs_err)
        mooseError("Flux normalization process failed! nekRS integrated flux: ", normalized_nek_flux,
          " MOOSE integrated flux: ",  *_flux_integral, ".\n\nThis may happen if the nekRS mesh "
          "is very different from that used in the App sending heat flux to nekRS and the "
          "nearest node transfer is only picking up zero values in the coupled App.");

      nekrs::copyFluxToDevice();
      break;
    }

    case ExternalProblem::Direction::FROM_EXTERNAL_APP:
    {
      if (!synchronizeOut())
      {
        _console << "Skipping temperature transfer out of nekRS, not at synchronization step" << std::endl;
        return;
      }

      _console << "Extracting nekRS temperature from boundary " << Moose::stringify(_boundary) << "... " << std::endl;

      // Get the temperature solution from nekRS. Note that nekRS performs a global communication
      // here such that each nekRS process has all the boundary temperature information. In
      // other words, regardless of which elements a nek rank owns, after calling nekrs::temperature,
      // every process knows the temperature on the boundary.
      nekrs::temperature(_temp_interpolation_matrix, _nek_mesh->order(), _needs_interpolation, _T);

      auto & solution = _aux->solution();
      auto sys_number = _aux->number();
      auto pid = _communicator.rank();

      for (unsigned int e = 0; e < _n_surface_elems; e++)
      {
        auto elem_ptr = _nek_mesh->elemPtr(e);

        for (unsigned int n = 0; n < _n_vertices_per_face; n++)
        {
          auto node_ptr = elem_ptr->node_ptr(n);

          // For each face vertex, we can only write into the MOOSE auxiliary fields if that
          // vertex is "owned" by the present MOOSE process.
          if (node_ptr->processor_id() == pid)
          {
            int node_index = _nek_mesh->NodeIndex(n);
            auto node_offset = e * _n_vertices_per_face + node_index;

            // get the DOF for the temperature auxiliary variable, then use it to set the
            // temperature in the auxiliary system
            auto dof_idx = node_ptr->dof_number(sys_number, _temp_var, 0);
            solution.set(dof_idx, _T[node_offset]);
          }
        }
      }

      solution.close();

      _console << "Interpolated temperature min/max values on interface: " <<
        minInterpolatedTemperature() << ", " << maxInterpolatedTemperature() << std::endl;

      break;
    }
    default:
      mooseError("Unhandled 'Transfer::DIRECTION' enum!");
  }
}

double
NekRSProblem::maxInterpolatedTemperature() const
{
  double maximum = std::numeric_limits<double>::min();

  int n_points = _n_surface_elems * _n_vertices_per_face;
  for (int i = 0; i < n_points; ++i)
    maximum = std::max(maximum, _T[i]);

  return maximum;
}

double
NekRSProblem::minInterpolatedTemperature() const
{
  double minimum = std::numeric_limits<double>::max();

  int n_points = _n_surface_elems * _n_vertices_per_face;
  for (int i = 0; i < n_points; ++i)
    minimum = std::min(minimum, _T[i]);

  return minimum;
}

void
NekRSProblem::addExternalVariables()
{
  auto var_params = _factory.getValidParams("MooseVariable");
  var_params.set<MooseEnum>("family") = "LAGRANGE";

  switch (_nek_mesh->order())
  {
    case surface::first:
      var_params.set<MooseEnum>("order") = "FIRST";
      break;
    case surface::second:
      var_params.set<MooseEnum>("order") = "SECOND";
      break;
    default:
      mooseError("Unhandled 'NekOrderEnum' in 'NekRSProblem'!");
  }

  // Because this temperature represents the reconstruction of nekRS's temperature
  // onto the NekMesh, we set the order to match the desired order of the surface.
  // Note that this does _not_ imply anything about the order of the temperature
  // variable in the MOOSE app (such as BISON) coupled to nekRS. This is just the
  // variable that nekRS writes into, and then MOOSE's transfer classes can handle
  // any additional interpolations needed from 'temp' into the receiving-app's fields.
  addAuxVariable("MooseVariable", "temp", var_params);
  _temp_var = _aux->getFieldVariable<Real>(0, "temp").number();

  // Likewise, because this flux represents the reconstruction of the flux variable
  // that becomes a boundary condition in the nekRS model, we set the order to match
  // the desired order of the surface. Note that this does _not_ imply anything
  // about the order of the surface flux in the MOOSE app (such as BISON) coupled
  // to nekRS. This is just the variable that nekRS reads from - MOOSE's transfer
  // classes handle any additional interpolations needed from the flux on the
  // sending app (such as BISON) into 'avg_flux'.
  addAuxVariable("MooseVariable", "avg_flux", var_params);
  _avg_flux_var = _aux->getFieldVariable<Real>(0, "avg_flux").number();
}
