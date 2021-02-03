#include "NekRSProblem.h"
#include "Moose.h"
#include "AuxiliarySystem.h"
#include "TimeStepper.h"
#include "NekInterface.h"

#include "nekrs.hpp"
#include "nekInterface/nekInterfaceAdapter.hpp"

registerMooseObject("NekApp", NekRSProblem);

bool NekRSProblem::_first = true;

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

  // the way the data transfers are detected depend on nekRS being a sub-application,
  // so these settings are not invalid if nekRS is the master app (though you could
  // relax this in the future by reversing the synchronization step identification
  // from the nekRS-subapp case to the nekRS-master app case - it's just not implemented yet).
  if (_app.isUltimateMaster())
    if (_minimize_transfers_in || _minimize_transfers_out)
      mooseError("The 'minimize_transfers_in' and 'minimize_transfers_out' capabilities "
        "require that nekRS is receiving and sending data to a master application, but "
        "in your case nekRS is the master application.");

  _nek_mesh = dynamic_cast<NekRSMesh*>(&mesh());

  if (!_nek_mesh)
    mooseError("Mesh for a 'NekRSProblem' must be of type 'NekRSMesh'! In your [Mesh] "
      "block, you should have 'type = NekRSMesh'");

  // boundary-specific data
  _boundary = _nek_mesh->boundary();
  _n_surface_elems = _nek_mesh->numSurfaceElems();
  _n_vertices_per_surface = _nek_mesh->numVerticesPerSurface();

  // volume-specific data
  _volume = _nek_mesh->volume();
  _n_volume_elems = _nek_mesh->numVolumeElems();
  _n_vertices_per_volume = _nek_mesh->numVerticesPerVolume();

  // generic data
  _n_elems = _nek_mesh->numElems();
  _n_vertices_per_elem = _nek_mesh->numVerticesPerElem();

  // Depending on the type of coupling, initialize various problem parameters
  if (_boundary && !_volume) // only boundary coupling
  {
    _incoming = "boundary heat flux";
    _outgoing = "boundary temperature";
    _n_points = _n_surface_elems * _n_vertices_per_surface;
    _T = (double*) calloc(_n_points, sizeof(double));
    _flux_face = (double *) calloc(_n_vertices_per_surface, sizeof(double));
  }
  else if (_volume && !_boundary) // only volume coupling
  {
    _incoming = "volume power density";
    _outgoing = "volume temperature and density";
    _n_points = _n_volume_elems * _n_vertices_per_volume;
    _T = (double*) calloc(_n_points, sizeof(double));
    _source_elem = (double*) calloc(_n_vertices_per_volume, sizeof(double));
  }
  else // both volume and boundary coupling
  {
    _incoming = "heat flux and power density";
    _outgoing = "volume temperature and density";
    _n_points = _n_volume_elems * _n_vertices_per_volume;
    _T = (double*) calloc(_n_points, sizeof(double));
    _source_elem = (double*) calloc(_n_vertices_per_volume, sizeof(double));
  }

  nekrs::initializeInterpolationMatrices(_nek_mesh->numQuadraturePoints1D());

  // we can save some effort for the low-order situations where the interpolation
  // matrix is the identity matrix (i.e. for which equi-spaced libMesh nodes are an
  // exact subset of the nekRS GLL points). This will happen for any first-order mesh,
  // and if a second-order mesh is used with a polynomial order of 2 in nekRS. Because
  // we pretty much always use a polynomial order greater than 2 in nekRS, let's just
  // check the first case because this will simplify our code in the nekrs::boundaryTemperature()
  // function. If you change this line, you MUST change the innermost if/else statement
  // in nekrs::temperature!
  _needs_interpolation = _nek_mesh->numQuadraturePoints1D() > 2;
}

NekRSProblem::~NekRSProblem()
{
  // write nekRS solution to output if not already written for this step
  if (!isOutputStep())
    nekrs::outfld(_time);

  if (_T) free(_T);
  if (_flux_face) free(_flux_face);
  if (_source_elem) free(_source_elem);
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

  if (_boundary)
    _flux_integral = &getPostprocessorValueByName("flux_integral");
  if (_volume)
    _source_integral = &getPostprocessorValueByName("source_integral");

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
  static bool first = true;

  if (_minimize_transfers_in)
  {
    // For the minimized incoming synchronization to work correctly, the value
    // of the incoming postprocessor must not be zero. We only need to check this for the very
    // first time we evaluate this function. This ensures that you don't accidentally set a
    // zero value as a default in the master application's postprocessor.
    if (first && *_transfer_in == false)
      mooseError("The default value for the 'transfer_in' postprocessor received by nekRS "
        "must not be false! Make sure that the master application's "
        "postprocessor is not zero.");

    if (*_transfer_in == false)
      synchronize = false;
    else
      setPostprocessorValueByName(*_transfer_in_name, false, 0);
  }

  first = false;
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

void
NekRSProblem::sendBoundaryHeatFluxToNek()
{
  _console << "Sending heat flux to nekRS boundary " << Moose::stringify(*_boundary) << "... " << std::endl;

  auto & solution = _aux->solution();
  auto sys_number = _aux->number();

  if (_first)
  {
    _serialized_solution->init(_aux->sys().n_dofs(), false, SERIAL);
    _first = false;
  }

  solution.localize(*_serialized_solution);

  auto & mesh = _nek_mesh->getMesh();
  const Real scale_squared = _nek_mesh->scaling() * _nek_mesh->scaling();

  for (unsigned int e = 0; e < _n_surface_elems; e++)
  {
    auto elem_ptr = mesh.elem_ptr(e);

    for (unsigned int n = 0; n < _n_vertices_per_surface; n++)
    {
      auto node_ptr = elem_ptr->node_ptr(n);

      // For each face, get the flux at the libMesh nodes. This will be passed into
      // nekRS, which will interpolate onto its GLL points. Because we are looping over
      // nodes from libMesh, we need to get the GLL index known by nekRS and use it to
      // determine the offset in the nekRS arrays.
      int node_index = _nek_mesh->boundaryNodeIndex(n);
      auto node_offset = e * _n_vertices_per_surface + node_index;

      auto dof_idx = node_ptr->dof_number(sys_number, _avg_flux_var, 0);
      _flux_face[node_index] = (*_serialized_solution)(dof_idx) * scale_squared;
    }

    // Now that we have the flux at the nodes of the NekRSMesh, we can interpolate them
    // onto the nekRS GLL points and then write into the flux scratch space.
    nekrs::flux(e, _nek_mesh->order(), _flux_face);
  }

  // Because the NekMesh may be quite different from that used in the app solving for
  // the heat flux, we will need to normalize the total flux on the nekRS side by the
  // total flux computed by the coupled MOOSE app.
  const double nek_flux = nekrs::fluxIntegral();
  const double moose_flux = *_flux_integral;

  _console << "Normalizing total nekRS flux of " << nek_flux << " to the conserved MOOSE "
    "value of " << moose_flux << "..." << std::endl;

  // If before normalization, there is a large difference between the nekRS imposed flux
  // and the MOOSE flux, this could mean that there is a poor match between the domains,
  // even if neither value is zero. For instance, if you forgot that the nekRS mesh is in
  // units of centimeters, but you're coupling to an app based in meters, the fluxes will
  // be very different from one another.
  if (moose_flux && (std::abs(nek_flux - moose_flux) / moose_flux) > 0.25)
    mooseDoOnce(mooseWarning("nekRS flux differs from MOOSE flux by more than 25\%! "
      "This could indicate that your geometries do not line up properly."));

  nekrs::normalizeFlux(moose_flux, nek_flux);

  // We can do an extra check here to make sure that the normalization was done correctly.
  // This will check if there was perhaps a disconnect in the boundaries nekRS thinks are
  // exchanging data vs. what is actually being sent.
  const double normalized_nek_flux = nekrs::fluxIntegral();
  bool high_rel_err = std::abs(normalized_nek_flux - moose_flux) / moose_flux > 1e-6;
  bool high_abs_err = std::abs(normalized_nek_flux - moose_flux) > 1e-6;

  if (high_rel_err || high_abs_err)
    mooseError("Flux normalization process failed! nekRS integrated flux: ", normalized_nek_flux,
      " MOOSE integrated flux: ", moose_flux, ".\n\nThis may happen if the nekRS mesh "
      "is very different from that used in the App sending heat flux to nekRS and the "
      "nearest node transfer is only picking up zero values in the coupled App.");
}

void
NekRSProblem::sendVolumeHeatSourceToNek()
{
  _console << "Sending heat source to nekRS volume..." << std::endl;

  auto & solution = _aux->solution();
  auto sys_number = _aux->number();

  if (_first)
  {
    _serialized_solution->init(_aux->sys().n_dofs(), false, SERIAL);
    _first = false;
  }

  solution.localize(*_serialized_solution);

  auto & mesh = _nek_mesh->getMesh();
  const Real scale_cubed = _nek_mesh->scaling() * _nek_mesh->scaling() * _nek_mesh->scaling();

  for (unsigned int e = 0; e < _n_volume_elems; e++)
  {
    auto elem_ptr = mesh.elem_ptr(e);

    for (unsigned int n = 0; n < _n_vertices_per_volume; n++)
    {
      auto node_ptr = elem_ptr->node_ptr(n);

      // For each element, get the heat source at the libMesh nodes. This will be passed into
      // nekRS, which will interpolate onto its GLL points. Because we are looping over
      // nodes from libMesh, we need to get the GLL index known by nekRS and use it to
      // determine the offset in the nekRS arrays.
      int node_index = _nek_mesh->volumeNodeIndex(n);
      auto node_offset = e * _n_vertices_per_volume + node_index;

      auto dof_idx = node_ptr->dof_number(sys_number, _heat_source_var, 0);
      _source_elem[node_index] = (*_serialized_solution)(dof_idx) * scale_cubed;
    }

    // Now that we have the heat source at the nodes of the NekRSMesh, we can interpolate them
    // onto the nekRS GLL points and then write into the source scratch space.
    nekrs::heat_source(e, _nek_mesh->order(), _source_elem);
  }

  // Because the NekMesh may be quite different from that used in the app solving for
  // the heat source, we will need to normalize the total source on the nekRS side by the
  // total source computed by the coupled MOOSE app.
  const double nek_source = nekrs::sourceIntegral();
  const double moose_source = *_source_integral;

  _console << "Normalizing total nekRS heat source of " << nek_source << " to the conserved MOOSE "
    "value of " << moose_source << "..." << std::endl;

  // If before normalization, there is a large difference between the nekRS imposed source
  // and the MOOSE source, this could mean that there is a poor match between the domains,
  // even if neither value is zero. For instance, if you forgot that the nekRS mesh is in
  // units of centimeters, but you're coupling to an app based in meters, the sources will
  // be very different from one another.
  if (moose_source && (std::abs(nek_source - moose_source) / moose_source) > 0.1)
    mooseDoOnce(mooseWarning("nekRS source differs from MOOSE source by more than 10\%! "
      "This could indicate that your geometries do not line up properly."));

  nekrs::normalizeHeatSource(moose_source, nek_source);

  // We can do an extra check here to make sure that the normalization was done correctly.
  // This will check if there was perhaps a disconnect in the boundaries nekRS thinks are
  // exchanging data vs. what is actually being sent.
  const double normalized_nek_source = nekrs::sourceIntegral();
  bool high_rel_err = std::abs(normalized_nek_source - moose_source) / moose_source > 1e-6;
  bool high_abs_err = std::abs(normalized_nek_source - moose_source) > 1e-6;

  if (high_rel_err || high_abs_err)
    mooseError("Heat source normalization process failed! nekRS integrated heat source: ", normalized_nek_source,
      " MOOSE integrated heat source: ", moose_source, ".\n\nThis may happen if the nekRS mesh "
      "is very different from that used in the App sending heat source to nekRS and the "
      "nearest node transfer is only picking up zero values in the coupled App.");
}

void
NekRSProblem::fillAuxVariable(const unsigned int var_number, const double * value)
{
  auto & solution = _aux->solution();
  auto sys_number = _aux->number();
  auto pid = _communicator.rank();

  for (unsigned int e = 0; e < _n_elems; e++)
  {
    auto elem_ptr = _nek_mesh->elemPtr(e);

    for (unsigned int n = 0; n < _n_vertices_per_elem; n++)
    {
      auto node_ptr = elem_ptr->node_ptr(n);

      // For each face vertex, we can only write into the MOOSE auxiliary fields if that
      // vertex is "owned" by the present MOOSE process.
      if (node_ptr->processor_id() == pid)
      {
        int node_index = _nek_mesh->nodeIndex(n);
        auto node_offset = e * _n_vertices_per_elem + node_index;

        // get the DOF for the temperature auxiliary variable, then use it to set the
        // temperature in the auxiliary system
        auto dof_idx = node_ptr->dof_number(sys_number, var_number, 0);
        solution.set(dof_idx, value[node_offset]);
      }
    }
  }

  solution.close();
}

void
NekRSProblem::getBoundaryTemperatureFromNek()
{
  _console << "Extracting nekRS temperature from boundary " << Moose::stringify(*_boundary) << "... " << std::endl;

  // Get the temperature solution from nekRS. Note that nekRS performs a global communication
  // here such that each nekRS process has all the boundary temperature information. In
  // other words, regardless of which elements a nek rank owns, after calling nekrs::temperature,
  // every process knows the temperature on the boundary.
  nekrs::boundaryTemperature(_nek_mesh->order(), _needs_interpolation, _T);
}

void
NekRSProblem::getVolumeTemperatureFromNek()
{
  _console << "Extracting nekRS temperature from volume... " << std::endl;

  // Get the temperature solution from nekRS. Note that nekRS performs a global communication
  // here such that each nekRS process has all the volume temperature information. In
  // other words, regardless of which elements a nek rank owns, after calling nekrs::temperature,
  // every process knows the temperature in the volume.
  nekrs::volumeTemperature(_nek_mesh->order(), _needs_interpolation, _T);
}

void NekRSProblem::syncSolutions(ExternalProblem::Direction direction)
{
  switch(direction)
  {
    case ExternalProblem::Direction::TO_EXTERNAL_APP:
    {
      if (!synchronizeIn())
      {
        _console << "Skipping " << _incoming << " transfer to nekRS, not at synchronization step" << std::endl;
        return;
      }

      if (_boundary && !_volume)
        sendBoundaryHeatFluxToNek();

      if (!_boundary && _volume)
        sendVolumeHeatSourceToNek();

      nekrs::copyScratchToDevice();

      break;
    }

    case ExternalProblem::Direction::FROM_EXTERNAL_APP:
    {
      if (!synchronizeOut())
      {
        _console << "Skipping " << _outgoing << " transfer out of nekRS, not at synchronization step" << std::endl;
        return;
      }

      if (_boundary && !_volume)
        getBoundaryTemperatureFromNek();

      if (!_boundary && _volume)
        getVolumeTemperatureFromNek();

      fillAuxVariable(_temp_var, _T);

      _console << "Interpolated temperature min/max values: " <<
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

  for (int i = 0; i < _n_points; ++i)
    maximum = std::max(maximum, _T[i]);

  return maximum;
}

double
NekRSProblem::minInterpolatedTemperature() const
{
  double minimum = std::numeric_limits<double>::max();

  for (int i = 0; i < _n_points; ++i)
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
    case order::first:
      var_params.set<MooseEnum>("order") = "FIRST";
      break;
    case order::second:
      var_params.set<MooseEnum>("order") = "SECOND";
      break;
    default:
      mooseError("Unhandled 'NekOrderEnum' in 'NekRSProblem'!");
  }

  // Because this temperature represents the reconstruction of nekRS's temperature
  // onto the NekRSMesh, we set the order to match the desired order of the mesh.
  // Note that this does _not_ imply anything about the order of the temperature
  // variable in the MOOSE app (such as BISON) coupled to nekRS. This is just the
  // variable that nekRS writes into, and then MOOSE's transfer classes can handle
  // any additional interpolations needed from 'temp' into the receiving-app's fields.
  addAuxVariable("MooseVariable", "temp", var_params);
  _temp_var = _aux->getFieldVariable<Real>(0, "temp").number();

  if (_boundary)
  {
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

  if (_volume)
  {
    addAuxVariable("MooseVariable", "heat_source", var_params);
    _heat_source_var = _aux->getFieldVariable<Real>(0, "heat_source").number();
  }
}
