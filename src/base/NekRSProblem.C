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

#include "NekRSProblem.h"
#include "Moose.h"
#include "AuxiliarySystem.h"
#include "TimeStepper.h"
#include "NekInterface.h"
#include "TimedPrint.h"
#include "MooseUtils.h"
#include "CardinalUtils.h"

#include "nekrs.hpp"
#include "nekInterface/nekInterfaceAdapter.hpp"

registerMooseObject("CardinalApp", NekRSProblem);

bool NekRSProblem::_first = true;

InputParameters
NekRSProblem::validParams()
{
  InputParameters params = NekRSProblemBase::validParams();
  params.addParam<bool>("minimize_transfers_in", false, "Whether to only synchronize nekRS "
    "for the direction TO_EXTERNAL_APP on multiapp synchronization steps");
  params.addParam<bool>("minimize_transfers_out", false, "Whether to only synchronize nekRS "
    "for the direction FROM_EXTERNAL_APP on multiapp synchronization steps");
  params.addParam<bool>("moving_mesh", false, "Whether we have a moving mesh problem or not");

  params.addParam<bool>("has_heat_source", true, "Whether a heat source will be applied to the NekRS domain. "
    "We allow this to be turned off so that we don't need to add an OCCA source kernel if we know the "
    "heat source in the NekRS domain is zero anyways (such as if NekRS only solves for the fluid and we have solid fuel).");

  params.addParam<PostprocessorName>("min_T", "If provided, postprocessor used to limit the minimum "
    "temperature (in dimensional form) in the nekRS problem");
  params.addParam<PostprocessorName>("max_T", "If provided, postprocessor used to limit the maximum "
    "temperature (in dimensional form) in the nekRS problem");
  return params;
}

NekRSProblem::NekRSProblem(const InputParameters &params) : NekRSProblemBase(params),
    _serialized_solution(NumericVector<Number>::build(_communicator).release()),
    _moving_mesh(getParam<bool>("moving_mesh")),
    _minimize_transfers_in(getParam<bool>("minimize_transfers_in")),
    _minimize_transfers_out(getParam<bool>("minimize_transfers_out")),
    _has_heat_source(getParam<bool>("has_heat_source"))
{
  // will be implemented soon
  if (_moving_mesh)
  {
    if (_nondimensional)
      mooseError("Moving mesh features are not yet implemented for a non-dimensional nekRS case!");

    if (!_nek_mesh->getMesh().is_replicated())
      mooseError("Distributed mesh features are not yet implemented for moving mesh cases!");
  }

  // the way the data transfers are detected depend on nekRS being a sub-application,
  // so these settings are not invalid if nekRS is the master app (though you could
  // relax this in the future by reversing the synchronization step identification
  // from the nekRS-subapp case to the nekRS-master app case - it's just not implemented yet).
  if (_app.isUltimateMaster())
    if (_minimize_transfers_in || _minimize_transfers_out)
      mooseError("The 'minimize_transfers_in' and 'minimize_transfers_out' capabilities "
        "require that nekRS is receiving and sending data to a master application, but "
        "in your case nekRS is the master application.");

  // Depending on the type of coupling, initialize various problem parameters
  if (_boundary && !_volume) // only boundary coupling
  {
    _incoming = "boundary heat flux";
    _outgoing = "boundary temperature";
    _flux_face = (double *) calloc(_n_vertices_per_surface, sizeof(double));
  }
  else if (_volume && !_boundary) // only volume coupling
  {
    _incoming = "volume power density";
    _outgoing = "volume temperature";
    _source_elem = (double*) calloc(_n_vertices_per_volume, sizeof(double));
  }
  else // both volume and boundary coupling
  {
    _incoming = "boundary heat flux and volume power density";
    _outgoing = "volume temperature";
    _flux_elem = (double *) calloc(_n_vertices_per_volume, sizeof(double));
    _source_elem = (double*) calloc(_n_vertices_per_volume, sizeof(double));
  }

  if (_moving_mesh)
  {
    _incoming += " and mesh displacement";

    if (_boundary)
    {
      mooseError("Mesh displacement not supported in boundary coupling!");
      // pending release of mesh solver in nekRS...
    }
    else if (_volume)
    {
      nekrs::save_initial_mesh();
      _displacement_x = (double *) calloc(_n_vertices_per_volume, sizeof(double));
      _displacement_y = (double *) calloc(_n_vertices_per_volume, sizeof(double));
      _displacement_z = (double *) calloc(_n_vertices_per_volume, sizeof(double));
    }
  }

  // regardless of the boundary/volume coupling, we will always exchange temperature
  _T = (double*) calloc(_n_points, sizeof(double));
}

NekRSProblem::~NekRSProblem()
{
  nekrs::freeScratch();

  freePointer(_T);
  freePointer(_flux_face);
  freePointer(_source_elem);
  freePointer(_flux_elem);

  freePointer(_displacement_x);
  freePointer(_displacement_y);
  freePointer(_displacement_z);
}

void
NekRSProblem::initialSetup()
{
  if (nekrs::buildOnly())
    return;

  NekRSProblemBase::initialSetup();

  // While we don't require nekRS to actually _solve_ for the temperature, we should
  // print a warning if there is no temperature solve. For instance, the check in
  // Nek makes sure that we have a [TEMPERATURE] block in the nekRS input file, but we
  // might still toggle the solver off by setting 'solver = none'. Warn the user if
  // the solve is turned off because this is really only a testing feature.
  bool has_temperature_solve = nekrs::hasTemperatureSolve();
  if (!has_temperature_solve)
    mooseWarning("By setting 'solver = none' for temperature in the .par file, nekRS "
      "will not solve for temperature.\n\nThe temperature transferred to MOOSE will remain "
      "fixed at its initial condition, and the heat flux and power transferred to nekRS will be unused.");

  // For boundary-based coupling, we should check that the correct flux boundary
  // condition is set on all of nekRS's boundaries. To avoid throwing this
  // error for test cases where we have a [TEMPERATURE] block but set its solve
  // to 'none', we also check whether we're actually computing for the temperature.
  auto boundary = _nek_mesh->boundary();
  if (boundary && has_temperature_solve)
  {
    for (const auto & b : *boundary)
      if (!nekrs::mesh::isHeatFluxBoundary(b))
      {
        const std::string type = nekrs::mesh::temperatureBoundaryType(b);
        mooseError("In order to send a boundary heat flux to nekRS, you must have a flux condition "
          "for each 'boundary' set in 'NekRSMesh'!\nBoundary " + std::to_string(b) + " is of type '" +
          type + "' instead of 'fixedGradient'.");
      }
  }

  // For volume-based coupling, we should check that there is a udf function providing
  // the source for the passive scalar equations (this is the analogue of the boundary
  // condition check for boundary-based coupling). NOTE: This check is imperfect, because
  // even if there is a source kernel, we cannot tell _which_ passive scalar equation that
  // it is applied to (we have source kernels for the RANS passive scalar equations, for instance).
  if (_nek_mesh->volume() && _has_heat_source)
    if (has_temperature_solve && !nekrs::hasHeatSourceKernel())
      mooseError("In order to send a heat source to nekRS, you must have an OCCA kernel "
        "for the source in the passive scalar equations!");

  if (_moving_mesh && !nekrs::hasMovingMesh())
    mooseError("In order for MOOSE to compute a mesh deformation in NekRS, you "
      "must have 'solver = user' in the [MESH] block!");

  if (_boundary)
    _flux_integral = &getPostprocessorValueByName("flux_integral");
  if (_volume && _has_heat_source)
    _source_integral = &getPostprocessorValueByName("source_integral");
  if (_minimize_transfers_in)
    _transfer_in = &getPostprocessorValueByName("transfer_in");

  if (isParamValid("min_T"))
  {
    auto name = getParam<PostprocessorName>("min_T");
    _min_T = &getPostprocessorValueByName(name);
  }

  if (isParamValid("max_T"))
  {
    auto name = getParam<PostprocessorName>("max_T");
    _max_T = &getPostprocessorValueByName(name);
  }

  // save initial mesh for moving mesh problems to match deformation in exodus output files
  if (_moving_mesh && !_disable_fld_file_output)
    nekrs::outfld(_timestepper->nondimensionalDT(_time));
}

void NekRSProblem::adjustNekSolution()
{
  // limit the temperature based on user settings
  bool limit_temperature = _min_T || _max_T;
  std::string msg;
  if (_min_T && !_max_T)
    msg = "Limiting nekRS temperature to above minimum temperature of " + Moose::stringify(*_min_T);
  if (_max_T && !_min_T)
    msg = "Limiting nekRS temperature to below maximum temperature of " + Moose::stringify(*_max_T);
  if (_max_T && _min_T)
    msg = "Limiting nekRS temperature to within the range [" + Moose::stringify(*_min_T) + ", " +
      Moose::stringify(*_max_T);

  if (limit_temperature)
  {
    _console << msg << std::endl;
    nekrs::limitTemperature(_min_T, _max_T);
  }
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
      setPostprocessorValueByName("transfer_in", false, 0);
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
  auto & solution = _aux->solution();
  auto sys_number = _aux->number();

  if (_first)
  {
    _serialized_solution->init(_aux->sys().n_dofs(), false, SERIAL);
    _first = false;
  }

  solution.localize(*_serialized_solution);

  auto & mesh = _nek_mesh->getMesh();

  {
    _console << "Sending heat flux to NekRS boundary " << Moose::stringify(*_boundary) << std::endl;

    if (!_volume)
    {
      for (unsigned int e = 0; e < _n_surface_elems; e++)
      {
        auto elem_ptr = mesh.query_elem_ptr(e);

        // Only work on elements we can find on our local chunk of a
        // distributed mesh
        if (!elem_ptr)
        {
          libmesh_assert(!mesh.is_serial());
          continue;
        }

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
          _flux_face[node_index] = (*_serialized_solution)(dof_idx) / nekrs::solution::referenceFlux();
        }

        // Now that we have the flux at the nodes of the NekRSMesh, we can interpolate them
        // onto the nekRS GLL points
        nekrs::flux(e, _nek_mesh->order(), _flux_face);
      }
    }
    else if (_volume)
    {
      // For the case of a boundary-only coupling, we could just loop over the elements on
      // the boundary of interest and write (carefully) into the volume nrs-usrwrk array. Now,
      // our flux variable is defined over the entire volume (maybe the MOOSE transfer only sent
      // meaningful values to the coupling boundaries), so we need to do a volume interpolation
      // of the flux into nrs->usrwrk, rather than a face interpolation. This could definitely be
      // optimized in the future to truly only just write the boundary values into the nekRS
      // scratch space rather than the volume values, but it looks right now that our biggest
      // expense occurs in the MOOSE transfer system, not these transfers internally to nekRS.
      for (unsigned int e = 0; e < _n_volume_elems; ++e)
      {
        int n_faces_on_boundary = nekrs::mesh::facesOnBoundary(e);

        auto elem_ptr = mesh.query_elem_ptr(e);

        // Only work on elements we can find on our local chunk of a
        // distributed mesh
        if (!elem_ptr)
          {
            libmesh_assert(!mesh.is_serial());
            continue;
          }

        // though the flux is a volume field, the only meaningful values are on the coupling
        // boundaries, so we can just skip this interpolation if this volume element isn't on
        // a coupling boundary, because that flux data isn't used anyways
        if (n_faces_on_boundary > 0)
        {
          auto elem_ptr = mesh.elem_ptr(e);

          for (unsigned int n = 0; n < _n_vertices_per_volume; ++n)
          {
            auto node_ptr = elem_ptr->node_ptr(n);

            // For each element, get the flux at the libMesh nodes. This will be passed into
            // nekRS, which will interpolate onto its GLL points. Because we are looping over
            // nodes from libMesh, we need to get the GLL index known by nekRS and use it to
            // determine the offset in the nekRS arrays.
            int node_index = _nek_mesh->volumeNodeIndex(n);
            auto node_offset = e * _n_vertices_per_volume + node_index;
            auto dof_idx = node_ptr->dof_number(sys_number, _avg_flux_var, 0);
            _flux_elem[node_index] = (*_serialized_solution)(dof_idx) / nekrs::solution::referenceFlux();
          }

          // Now that we have the flux at the nodes of the NekRSMesh, we can interpolate them
          // onto the nekRS GLL points
          nekrs::writeVolumeSolution(e, _nek_mesh->order(), field::flux, _flux_elem);
        }
      }
    }
  }

  // Because the NekRSMesh may be quite different from that used in the app solving for
  // the heat flux, we will need to normalize the total flux on the nekRS side by the
  // total flux computed by the coupled MOOSE app. For this and the next check of the
  // flux integral, we need to scale the integral back up again to the dimensional form
  // for the sake of comparison.
  const Real scale_squared = _nek_mesh->scaling() * _nek_mesh->scaling();
  const double nek_flux = nekrs::fluxIntegral();
  const double moose_flux = *_flux_integral;

  // For the sake of printing diagnostics to the screen regarding the flux normalization,
  // we first scale the nek flux by any unit changes and then by the reference flux.
  const double nek_flux_print_mult = scale_squared * nekrs::solution::referenceFlux();
  double normalized_nek_flux = 0.0;
  bool successful_normalization;

  _console << "Normalizing total NekRS flux of " << Moose::stringify(nek_flux * nek_flux_print_mult) <<
    " to the conserved MOOSE value of " << Moose::stringify(moose_flux) << std::endl;

  successful_normalization = nekrs::normalizeFlux(moose_flux, nek_flux, normalized_nek_flux);

  // If before normalization, there is a large difference between the nekRS imposed flux
  // and the MOOSE flux, this could mean that there is a poor match between the domains,
  // even if neither value is zero. For instance, if you forgot that the nekRS mesh is in
  // units of centimeters, but you're coupling to an app based in meters, the fluxes will
  // be very different from one another.
  if (moose_flux && (std::abs(nek_flux * nek_flux_print_mult - moose_flux) / moose_flux) > 0.25)
    mooseDoOnce(mooseWarning("nekRS flux differs from MOOSE flux by more than 25\%! "
      "This could indicate that your geometries do not line up properly."));

  if (!successful_normalization)
    mooseError("Flux normalization process failed! nekRS integrated flux: ", normalized_nek_flux,
      " MOOSE integrated flux: ", moose_flux, ".\n\nThis may happen if the nekRS mesh "
      "is very different from that used in the App sending heat flux to nekRS and the "
      "nearest node transfer is only picking up zero values in the coupled App.");
}

void
NekRSProblem::sendVolumeDeformationToNek()
{
  auto & solution = _aux->solution();
  auto sys_number = _aux->number();

  if (_first)
  {
    _serialized_solution->init(_aux->sys().n_dofs(), false, SERIAL);
    _first = false;
  }

  solution.localize(*_serialized_solution);

  auto & mesh = _nek_mesh->getMesh();

  _console << "Sending volume deformation to NekRS" << std::endl;

  for (unsigned int e = 0; e < _n_volume_elems; e++)
  {
    auto elem_ptr = mesh.query_elem_ptr(e);

    // Only work on elements we can find on our local chunk of a
    // distributed mesh
    if (!elem_ptr)
      {
        libmesh_assert(!mesh.is_serial());
        continue;
      }

    for (unsigned int n = 0; n < _n_vertices_per_volume; n++)
    {
      auto node_ptr = elem_ptr->node_ptr(n);

      // For each face, get the displacement at the libMesh nodes. This will be passed into
      // nekRS, which will interpolate onto its GLL points. Because we are looping over
      // nodes from libMesh, we need to get the GLL index known by nekRS and use it to
      // determine the offset in the nekRS arrays.
      int node_index = _nek_mesh->volumeNodeIndex(n);
      auto node_offset = e * _n_vertices_per_volume + node_index;
      auto dof_idx1 = node_ptr->dof_number(sys_number, _disp_x_var, 0);
      auto dof_idx2 = node_ptr->dof_number(sys_number, _disp_y_var, 0);
      auto dof_idx3 = node_ptr->dof_number(sys_number, _disp_z_var, 0);
      _displacement_x[node_index] = (*_serialized_solution)(dof_idx1);
      _displacement_y[node_index] = (*_serialized_solution)(dof_idx2);
      _displacement_z[node_index] = (*_serialized_solution)(dof_idx3);
    }

    // Now that we have the displacement at the nodes of the NekRSMesh, we can interpolate them
    // onto the nekRS GLL points
    nekrs::writeVolumeSolution(e, _nek_mesh->order(), field::x_displacement, _displacement_x);
    nekrs::writeVolumeSolution(e, _nek_mesh->order(), field::y_displacement, _displacement_y);
    nekrs::writeVolumeSolution(e, _nek_mesh->order(), field::z_displacement, _displacement_z);
  }
}

void
NekRSProblem::sendVolumeHeatSourceToNek()
{
  auto & solution = _aux->solution();
  auto sys_number = _aux->number();

  if (_first)
  {
    _serialized_solution->init(_aux->sys().n_dofs(), false, SERIAL);
    _first = false;
  }

  solution.localize(*_serialized_solution);

  auto & mesh = _nek_mesh->getMesh();

  {
    _console << "Sending volumetric heat source to NekRS" << std::endl;

    for (unsigned int e = 0; e < _n_volume_elems; e++)
    {
      auto elem_ptr = mesh.query_elem_ptr(e);

      // Only work on elements we can find on our local chunk of a
      // distributed mesh
      if (!elem_ptr)
        {
          libmesh_assert(!mesh.is_serial());
          continue;
        }

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
        _source_elem[node_index] = (*_serialized_solution)(dof_idx) / nekrs::solution::referenceSource();
      }

      // Now that we have the heat source at the nodes of the NekRSMesh, we can interpolate them
      // onto the nekRS GLL points
      nekrs::writeVolumeSolution(e, _nek_mesh->order(), field::heat_source, _source_elem);
    }
  }

  // Because the NekRSMesh may be quite different from that used in the app solving for
  // the heat source, we will need to normalize the total source on the nekRS side by the
  // total source computed by the coupled MOOSE app.
  const Real scale_cubed = _nek_mesh->scaling() * _nek_mesh->scaling() * _nek_mesh->scaling();
  const double nek_source = nekrs::sourceIntegral();
  const double moose_source = *_source_integral;

  // For the sake of printing diagnostics to the screen regarding source normalization,
  // we first scale the nek source by any unit changes and then by the reference source
  const double nek_source_print_mult = scale_cubed * nekrs::solution::referenceSource();
  double normalized_nek_source = 0.0;
  bool successful_normalization;

  _console << "Normalizing total NekRS heat source of " << Moose::stringify(nek_source * nek_source_print_mult) <<
    " to the conserved MOOSE value of " + Moose::stringify(moose_source) << std::endl;

  // Any unit changes (for DIMENSIONAL nekRS runs) are automatically accounted for
  // here because moose_source is an integral on the MOOSE mesh, while nek_source is
  // an integral on the nek mesh
  successful_normalization = nekrs::normalizeHeatSource(moose_source, nek_source, normalized_nek_source);

  // If before normalization, there is a large difference between the nekRS imposed source
  // and the MOOSE source, this could mean that there is a poor match between the domains,
  // even if neither value is zero. For instance, if you forgot that the nekRS mesh is in
  // units of centimeters, but you're coupling to an app based in meters, the sources will
  // be very different from one another.
  if (moose_source && (std::abs(nek_source * nek_source_print_mult - moose_source) / moose_source) > 0.25)
    mooseDoOnce(mooseWarning("nekRS source differs from MOOSE source by more than 25\%! "
      "This could indicate that your geometries do not line up properly."));

  if (!successful_normalization)
    mooseError("Heat source normalization process failed! nekRS integrated heat source: ", normalized_nek_source,
      " MOOSE integrated heat source: ", moose_source, ".\n\nThis may happen if the nekRS mesh "
      "is very different from that used in the App sending heat source to nekRS and the "
      "nearest node transfer is only picking up zero values in the coupled App.");
}

void
NekRSProblem::getBoundaryTemperatureFromNek()
{
  _console << "Extracting NekRS temperature from boundary " << Moose::stringify(*_boundary) << std::endl;

  // Get the temperature solution from nekRS. Note that nekRS performs a global communication
  // here such that each nekRS process has all the boundary temperature information. That is,
  // every process knows the full boundary temperature solution
  nekrs::boundarySolution(_nek_mesh->order(), _needs_interpolation, field::temperature, _T);
}

void
NekRSProblem::getVolumeTemperatureFromNek()
{
  _console << "Extracting NekRS temperature volume" << std::endl;

  // Get the temperature solution from nekRS. Note that nekRS performs a global communication
  // here such that each nekRS process has all the volume temperature information. In
  // other words, regardless of which elements a nek rank owns, after calling nekrs::temperature,
  // every process knows the temperature in the volume.
  nekrs::volumeSolution(_nek_mesh->order(), _needs_interpolation, field::temperature, _T);
}

void NekRSProblem::syncSolutions(ExternalProblem::Direction direction)
{
  if (nekrs::buildOnly())
    return;

  switch(direction)
  {
    case ExternalProblem::Direction::TO_EXTERNAL_APP:
    {
      if (!synchronizeIn())
      {
        _console << "Skipping " << _incoming << " transfer to nekRS, not at synchronization step" << std::endl;
        return;
      }

      if (_boundary)
        sendBoundaryHeatFluxToNek();

      if (_volume && _has_heat_source)
        sendVolumeHeatSourceToNek();

      // copy the boundary heat flux and/or volume heat source in the scratch space to device
      nekrs::copyScratchToDevice();

      if (_moving_mesh)
      {
        if (_volume)
        {
          sendVolumeDeformationToNek();
          nekrs::copyDeformationToDevice();
        }

        // no boundary-based mesh movement available in nekRS yet
      }

      break;
    }

    case ExternalProblem::Direction::FROM_EXTERNAL_APP:
    {
      if (!synchronizeOut())
      {
        _console << "Skipping " << _outgoing << " transfer out of nekRS, not at synchronization step" << std::endl;
        return;
      }

      if (!_volume)
        getBoundaryTemperatureFromNek();

      if (_volume)
        getVolumeTemperatureFromNek();

      // for boundary-only coupling, this fills a variable on a boundary mesh; otherwise,
      // this fills a variable on a volume mesh (because we will want a volume temperature for
      // neutronics feedback, and we can still get a temperature boundary condition from a volume set)
      fillAuxVariable(_temp_var, _T);

      _console << " Interpolated temperature min/max values: " <<
        minInterpolatedTemperature() << ", " << maxInterpolatedTemperature() << std::endl;

      // extract all outputs (except temperature, which we did separately here). We could
      // have simply called the base class NekRSProblemBase::syncSolutions to do this, but
      // putting this here lets us use a consistent setting for the minimize transfers feature,
      // if used (otherwise, the 'temp' variable could be extracted on a different frequency
      // than other specifications, such as pressure or mu_t.
      extractOutputs();

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
  NekRSProblemBase::addExternalVariables();
  auto var_params = getExternalVariableParameters();

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

    // add the postprocessor that receives the flux integral for normalization
    auto pp_params = _factory.getValidParams("Receiver");
    addPostprocessor("Receiver", "flux_integral", pp_params);
  }

  if (_volume && _has_heat_source)
  {
    addAuxVariable("MooseVariable", "heat_source", var_params);
    _heat_source_var = _aux->getFieldVariable<Real>(0, "heat_source").number();

    // add the postprocessor that receives the source integral for normalization
    auto pp_params = _factory.getValidParams("Receiver");
    addPostprocessor("Receiver", "source_integral", pp_params);
  }

  // add the displacement aux variables from the solid mechanics solver; these will
  // be needed regardless of whether the displacement is boundary- or volume-based
  if (_moving_mesh)
  {
    addAuxVariable("MooseVariable", "disp_x", var_params);
    _disp_x_var = _aux->getFieldVariable<Real>(0, "disp_x").number();

    addAuxVariable("MooseVariable", "disp_y", var_params);
    _disp_y_var = _aux->getFieldVariable<Real>(0, "disp_y").number();

    addAuxVariable("MooseVariable", "disp_z", var_params);
    _disp_z_var = _aux->getFieldVariable<Real>(0, "disp_z").number();
  }

  if (_minimize_transfers_in)
  {
    auto pp_params = _factory.getValidParams("Receiver");
    addPostprocessor("Receiver", "transfer_in", pp_params);
  }
}
