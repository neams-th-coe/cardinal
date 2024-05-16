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

#include "NekRSProblem.h"
#include "DisplacedProblem.h"

#include "nekInterface/nekInterfaceAdapter.hpp"

registerMooseObject("CardinalApp", NekRSProblem);

extern nekrs::usrwrkIndices indices;

InputParameters
NekRSProblem::validParams()
{
  InputParameters params = NekRSProblemBase::validParams();

  params.addParam<bool>("has_heat_source",
                        true,
                        "Whether a heat source will be applied to the NekRS domain. "
                        "We allow this to be turned off so that we don't need to add an OCCA "
                        "source kernel if we know the heat source in the NekRS domain is zero.");

  params.addRangeCheckedParam<Real>("normalization_abs_tol", 1e-8, "normalization_abs_tol > 0",
    "Absolute tolerance for checking if the boundary heat flux and volumetric heat sources "
    "sent from MOOSE to NekRS are re-normalized correctly");

  params.addRangeCheckedParam<Real>("normalization_rel_tol", 1e-5, "normalization_rel_tol > 0",
    "Absolute tolerance for checking if the boundary heat flux and volumetric heat sources "
    "sent from MOOSE to NekRS are re-normalized correctly");

  params.addParam<PostprocessorName>("min_T",
                                     "If provided, postprocessor used to limit the minimum "
                                     "temperature (in dimensional form) in the nekRS problem");
  params.addParam<PostprocessorName>("max_T",
                                     "If provided, postprocessor used to limit the maximum "
                                     "temperature (in dimensional form) in the nekRS problem");

  params.addParam<bool>("conserve_flux_by_sideset", false,
    "Whether to conserve the heat flux by individual sideset (as opposed to lumping all sidesets "
    "together). Setting this option to true requires syntax changes in the input file to use "
    "vector postprocessors, and places restrictions on how the sidesets are set up.");
  return params;
}

NekRSProblem::NekRSProblem(const InputParameters & params)
  : NekRSProblemBase(params),
    _has_heat_source(getParam<bool>("has_heat_source")),
    _conserve_flux_by_sideset(getParam<bool>("conserve_flux_by_sideset")),
    _abs_tol(getParam<Real>("normalization_abs_tol")),
    _rel_tol(getParam<Real>("normalization_rel_tol"))
{
  nekrs::setAbsoluteTol(getParam<Real>("normalization_abs_tol"));
  nekrs::setRelativeTol(getParam<Real>("normalization_rel_tol"));

  if (!_boundary)
    checkUnusedParam(params, "conserve_flux_by_sideset", "coupling NekRS solely through a volume mesh mirror");

  // Determine the usrwrk indexing; the ordering will always be as
  // follows (except that unused terms will be deleted if not needed for coupling)
  //   flux              (if _boundary is true)
  //   heat_source       (if _volume is true and _has_heat_source is true)
  //   mesh_velocity_x   (if nekrs::hasBlendingSolver() is true)
  //   mesh_velocity_y   (if nekrs::hasBlendingSolver() is true)
  //   mesh_velocity_z   (if nekrs::hasBlendingSolver() is true)
  int start = _usrwrk_indices.size();
  if (_boundary)
  {
    indices.flux = start++ * nekrs::scalarFieldOffset();
    _usrwrk_indices.push_back("flux");
  }

  if (_volume && _has_heat_source)
  {
    indices.heat_source = start++ * nekrs::scalarFieldOffset();
    _usrwrk_indices.push_back("heat_source");
  }

  if (nekrs::hasBlendingSolver())
  {
    indices.mesh_velocity_x = start++ * nekrs::scalarFieldOffset();
    indices.mesh_velocity_y = start++ * nekrs::scalarFieldOffset();
    indices.mesh_velocity_z = start++ * nekrs::scalarFieldOffset();
    _usrwrk_indices.push_back("mesh_velocity_x");
    _usrwrk_indices.push_back("mesh_velocity_y");
    _usrwrk_indices.push_back("mesh_velocity_z");
  }

  _minimum_scratch_size_for_coupling = _usrwrk_indices.size() - _first_reserved_usrwrk_slot;

  if (nekrs::hasMovingMesh())
  {
    // will be implemented soon
    if (!_nek_mesh->getMesh().is_replicated())
      mooseError("Distributed mesh features are not yet implemented for moving mesh cases!");

    if (!_app.actionWarehouse().displacedMesh())
      mooseError("Moving mesh problems require displacements in the [Mesh] block!");
  }

  if (_app.actionWarehouse().displacedMesh() && !nekrs::hasMovingMesh())
      mooseWarning("Your NekRSMesh has 'displacements', but '" + _casename + ".par' does not have a\n"
        "solver in the [MESH] block! The displacements transferred to NekRS will be unused.");

  if (nekrs::hasMovingMesh() && _nek_mesh->exactMirror())
    mooseError("An exact mesh mirror is not yet implemented for the boundary mesh solver.");

  int n_per_surf = _nek_mesh->exactMirror() ?
    std::pow(_nek_mesh->nekNumQuadraturePoints1D(), 2.0) : _n_vertices_per_surface;
  int n_per_vol = _nek_mesh->exactMirror() ?
    std::pow(_nek_mesh->nekNumQuadraturePoints1D(), 3.0) : _n_vertices_per_volume;

  // Depending on the type of coupling, initialize various problem parameters
  if (_boundary && !_volume) // only boundary coupling
    _flux_face = (double *)calloc(n_per_surf, sizeof(double));
  else if (_volume && !_boundary) // only volume coupling
    _source_elem = (double *)calloc(n_per_vol, sizeof(double));
  else // both volume and boundary coupling
  {
    _flux_elem = (double *)calloc(n_per_vol, sizeof(double));
    _source_elem = (double *)calloc(n_per_vol, sizeof(double));
  }

  if (nekrs::hasMovingMesh())
  {
    int n_entries = _volume ? n_per_vol : n_per_surf;
    _displacement_x = (double *)calloc(n_entries, sizeof(double));
    _displacement_y = (double *)calloc(n_entries, sizeof(double));
    _displacement_z = (double *)calloc(n_entries, sizeof(double));

    if (nekrs::hasBlendingSolver())
      _mesh_velocity_elem = (double *)calloc(n_entries, sizeof(double));
  }

  // regardless of the boundary/volume coupling, we will always exchange temperature
  _T = (double *)calloc(_n_points, sizeof(double));
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
  freePointer(_mesh_velocity_elem);
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
    mooseWarning(
        "By setting 'solver = none' for temperature in '" + _casename +
        ".par', nekRS "
        "will not solve for temperature.\n\nThe temperature transferred to MOOSE will remain "
        "fixed at its initial condition, and the heat flux\nand power transferred to nekRS will be "
        "unused.");

  // For boundary-based coupling, we should check that the correct flux boundary
  // condition is set on all of nekRS's boundaries. To avoid throwing this
  // error for test cases where we have a [TEMPERATURE] block but set its solve
  // to 'none', we also check whether we're actually computing for the temperature.
  auto boundary = _nek_mesh->boundary();
  if (boundary && has_temperature_solve)
  {
    for (const auto & b : *boundary)
      if (!nekrs::isHeatFluxBoundary(b))
      {
        const std::string type = nekrs::temperatureBoundaryType(b);
        mooseError("In order to send a boundary heat flux to nekRS, you must have a flux condition "
                   "for each 'boundary' set in 'NekRSMesh'!\nBoundary " +
                   std::to_string(b) + " is of type '" + type + "' instead of 'fixedGradient'.");
      }
  }

  if (!boundary && nekrs::hasBlendingSolver())
    mooseError("'" + _casename +
               ".par' has a solver in the [MESH] block. This solver uses\n"
               "boundary displacement values from MOOSE to move the NekRS mesh. Please indicate\n"
               "the 'boundary' for which mesh motion is coupled from MOOSE to NekRS.");

  if (nekrs::hasBlendingSolver())
  {
    bool has_one_mv_bc = false;
    for (const auto & b : *boundary)
    {
      if (nekrs::isMovingMeshBoundary(b))
      {
        has_one_mv_bc = true;
        break;
      }
    }

    if (!has_one_mv_bc)
      mooseError("For boundary-coupled moving mesh problems, you need at least one "
                 "boundary in '" +
                 _casename +
                 ".par'\nto be of the type 'codedFixedValue'"
                 " in the [MESH] block.");
  }

  // For volume-based coupling, we should check that there is a udf function providing
  // the source for the passive scalar equations (this is the analogue of the boundary
  // condition check for boundary-based coupling). NOTE: This check is imperfect, because
  // even if there is a source kernel, we cannot tell _which_ passive scalar equation that
  // it is applied to (we have source kernels for the RANS passive scalar equations, for instance).
  if (_nek_mesh->volume() && _has_heat_source)
    if (has_temperature_solve && !nekrs::hasHeatSourceKernel())
      mooseError(
          "In order to send a heat source to nekRS, you must have an OCCA kernel "
          "for the source\nin the passive scalar equations! If you don't have an energy source\n"
          "in your NekRS domain, you can disable this error with 'has_heat_source = false'.");

  if (!_volume && nekrs::hasUserMeshSolver())
    mooseError("'" + _casename + ".par' has 'solver = user' in the [MESH] block. With this solver,\n"
               "displacement values are sent to every GLL point in NekRS's volume. If you only are building\n"
               "a boundary mesh mirror, it's possible that some displacement values could result\n"
               "in negative Jacobians if a sideset moves beyond the bounds of an undeformed element.\n"
               "To eliminate this possibility, please enable 'volume = true' for NekRSMesh and send a\n"
               "whole-domain displacement to NekRS.");

  if (_boundary)
  {
    if (_conserve_flux_by_sideset)
      _flux_integral_vpp = &getVectorPostprocessorValueByName("flux_integral", "value");
    else
      _flux_integral = &getPostprocessorValueByName("flux_integral");
  }

  if (_volume && _has_heat_source)
    _source_integral = &getPostprocessorValueByName("source_integral");

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
  if (nekrs::hasMovingMesh() && !_disable_fld_file_output)
    nekrs::outfld(_timestepper->nondimensionalDT(_time), _t_step);

  if (nekrs::hasBlendingSolver())
    _nek_mesh->initializePreviousDisplacements();

  if (nekrs::hasUserMeshSolver())
    _nek_mesh->saveInitialVolMesh();
}

void
NekRSProblem::adjustNekSolution()
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
          Moose::stringify(*_max_T) + "]";

  if (limit_temperature)
  {
    _console << msg << std::endl;
    nekrs::limitTemperature(_min_T, _max_T);
  }
}

void
NekRSProblem::sendBoundaryDeformationToNek()
{
  _console << "Sending boundary deformation to NekRS..." <<std::endl;

  if (!_volume)
  {
    for (unsigned int e = 0; e < _n_surface_elems; e++)
    {
      // We can only write into the nekRS scratch space if that face is "owned" by the current process
      if (nekrs::commRank() != _nek_mesh->boundaryCoupling().processor_id(e))
        continue;

      mapFaceDataToNekFace(e, _disp_x_var, 1.0, &_displacement_x);
      calculateMeshVelocity(e, field::mesh_velocity_x);
      writeBoundarySolution(e, field::mesh_velocity_x, _mesh_velocity_elem);

      mapFaceDataToNekFace(e, _disp_y_var, 1.0, &_displacement_y);
      calculateMeshVelocity(e, field::mesh_velocity_y);
      writeBoundarySolution(e, field::mesh_velocity_y, _mesh_velocity_elem);

      mapFaceDataToNekFace(e, _disp_z_var, 1.0, &_displacement_z);
      calculateMeshVelocity(e, field::mesh_velocity_z);
      writeBoundarySolution(e, field::mesh_velocity_z, _mesh_velocity_elem);
    }
  }
  else
  {
    for (unsigned int e = 0; e < _n_volume_elems; ++e)
    {
      // We can only write into the nekRS scratch space if that face is "owned" by the current process
      if (nekrs::commRank() != _nek_mesh->volumeCoupling().processor_id(e))
        continue;

      mapFaceDataToNekVolume(e, _disp_x_var, 1.0, &_displacement_x);
      calculateMeshVelocity(e, field::mesh_velocity_x);
      writeVolumeSolution(e, field::mesh_velocity_x, _mesh_velocity_elem);

      mapFaceDataToNekVolume(e, _disp_y_var, 1.0, &_displacement_y);
      calculateMeshVelocity(e, field::mesh_velocity_y);
      writeVolumeSolution(e, field::mesh_velocity_y, _mesh_velocity_elem);

      mapFaceDataToNekVolume(e, _disp_z_var, 1.0, &_displacement_z);
      calculateMeshVelocity(e, field::mesh_velocity_z);
      writeVolumeSolution(e, field::mesh_velocity_z, _mesh_velocity_elem);
    }
  }

  _displaced_problem->updateMesh();
}

void
NekRSProblem::sendBoundaryHeatFluxToNek()
{
  _console << "Sending heat flux to NekRS boundary " << Moose::stringify(*_boundary) << "..."
           << std::endl;

  if (!_volume)
  {
    for (unsigned int e = 0; e < _n_surface_elems; e++)
    {
      // We can only write into the nekRS scratch space if that face is "owned" by the current process
      if (nekrs::commRank() != _nek_mesh->boundaryCoupling().processor_id(e))
        continue;

      mapFaceDataToNekFace(e, _avg_flux_var, 1.0 / nekrs::referenceFlux(), &_flux_face);
      writeBoundarySolution(e, field::flux, _flux_face);
    }
  }
  else
  {
    for (unsigned int e = 0; e < _n_volume_elems; ++e)
    {
      // We can only write into the nekRS scratch space if that face is "owned" by the current process
      if (nekrs::commRank() != _nek_mesh->volumeCoupling().processor_id(e))
        continue;

      mapFaceDataToNekVolume(e, _avg_flux_var, 1.0 / nekrs::referenceFlux(), &_flux_elem);
      writeVolumeSolution(e, field::flux, _flux_elem);
    }
  }

  // Because the NekRSMesh may be quite different from that used in the app solving for
  // the heat flux, we will need to normalize the flux on the nekRS side by the
  // flux computed by the coupled MOOSE app. For this and the next check of the
  // flux integral, we need to scale the integral back up again to the dimensional form
  // for the sake of comparison.
  const Real scale_squared = _nek_mesh->scaling() * _nek_mesh->scaling();
  const double nek_flux_print_mult = scale_squared * nekrs::referenceFlux();

  // integrate the flux over each individual boundary
  std::vector<double> nek_flux_sidesets =
      nekrs::usrwrkSideIntegral(indices.flux, *_boundary, nek_mesh::all);

  bool successful_normalization;
  double normalized_nek_flux = 0.0;

  double total_moose_flux;

  if (_conserve_flux_by_sideset)
  {
    auto moose_flux = *_flux_integral_vpp;
    if (moose_flux.size() != _boundary->size())
      mooseError("The sideset flux reporter transferred to NekRS must have a length equal to the number\n"
        "of entries in 'boundary'! Please check the values written to the 'flux_integral' vector postprocessor.\n\n"
        "Length of reporter: ", moose_flux.size(), "\n",
        "Length of 'boundary': ", _boundary->size());

    for (std::size_t b = 0; b < _boundary->size(); ++b)
    {
      _console << "[boundary " << Moose::stringify((*_boundary)[b]) << "]: Normalizing NekRS flux of "
               << Moose::stringify(nek_flux_sidesets[b] * nek_flux_print_mult)
               << " to the conserved MOOSE value of " << Moose::stringify(moose_flux[b]) << std::endl;

      checkInitialFluxValues(nek_flux_sidesets[b], moose_flux[b]);
    }

    total_moose_flux = std::accumulate(moose_flux.begin(), moose_flux.end(), 0.0);

    // For the sake of printing diagnostics to the screen regarding the flux normalization,
    // we first scale the nek flux by any unit changes and then by the reference flux.
    successful_normalization = nekrs::normalizeFluxBySideset(
        _nek_mesh->boundaryCoupling(), *_boundary, moose_flux, nek_flux_sidesets, normalized_nek_flux);
  }
  else
  {
    auto moose_flux = *_flux_integral;
    const double nek_flux = std::accumulate(nek_flux_sidesets.begin(), nek_flux_sidesets.end(), 0.0);

    _console << "[boundary " << Moose::stringify(*_boundary)
             << "]: Normalizing total NekRS flux of "
             << Moose::stringify(nek_flux * nek_flux_print_mult)
             << " to the conserved MOOSE value of " << Moose::stringify(moose_flux) << std::endl;

    checkInitialFluxValues(nek_flux, moose_flux);

    total_moose_flux = moose_flux;

    // For the sake of printing diagnostics to the screen regarding the flux normalization,
    successful_normalization = nekrs::normalizeFlux(
        _nek_mesh->boundaryCoupling(), *_boundary, moose_flux, nek_flux, normalized_nek_flux);
  }

  if (!successful_normalization)
    mooseError("Flux normalization process failed! NekRS integrated flux: ",
               normalized_nek_flux,
               " MOOSE integrated flux: ",
               total_moose_flux, ".\n",
               "There are a few reason this might happen:\n\n"
               "- You have a mismatch between the NekRS mesh and the MOOSE mesh. Try visualizing the\n"
               "  meshes in Paraview by running your input files with the --mesh-only flag.\n\n"
               "- Your tolerances for comparing the re-normalized NekRS flux with the incoming MOOSE\n"
               "  flux are too tight. If the NekRS flux is acceptably close to the MOOSE flux, you can\n"
               "  try relaxing the 'normalization_abs_tol' and/or 'normalization_rel_tol' parameters\n\n"
               "- You forgot to send a flux VARIABLE to NekRS, in which case no matter what you try to\n"
               "  normalize by, the flux in NekRS is always zero.\n\n"
               "- If you set 'conserve_flux_by_sideset = true' and nodes are SHARED by boundaries\n"
               "  (like on corners between sidesets), you will end up renormalizing those shared nodes\n"
               "  once per sideset that they lie on. There is no guarantee that the total imposed flux\n"
               "  would be preserved.");
}

void
NekRSProblem::checkInitialFluxValues(const Real & nek_flux, const Real & moose_flux) const
{
  const Real scale_squared = _nek_mesh->scaling() * _nek_mesh->scaling();
  const double nek_flux_print_mult = scale_squared * nekrs::referenceFlux();

  // If before normalization, there is a large difference between the nekRS imposed flux
  // and the MOOSE flux, this could mean that there is a poor match between the domains,
  // even if neither value is zero. For instance, if you forgot that the nekRS mesh is in
  // units of centimeters, but you're coupling to an app based in meters, the fluxes will
  // be very different from one another.
  if (moose_flux && (std::abs(nek_flux * nek_flux_print_mult - moose_flux) / moose_flux) > 0.25)
    mooseDoOnce(mooseWarning("NekRS flux differs from MOOSE flux by more than 25\%! This might indicate that your\n"
                             "geometries don't line up properly or something else is wrong with the flux transfer."));
}

void
NekRSProblem::sendVolumeDeformationToNek()
{
  _console << "Sending volume deformation to NekRS" << std::endl;

  for (unsigned int e = 0; e < _n_volume_elems; e++)
  {
    // We can only write into the nekRS scratch space if that face is "owned" by the current process
    if (nekrs::commRank() != _nek_mesh->volumeCoupling().processor_id(e))
      continue;

    mapVolumeDataToNekVolume(e, _disp_x_var, 1.0 / _L_ref, &_displacement_x);
    writeVolumeSolution(e, field::x_displacement, _displacement_x, &(_nek_mesh->nek_initial_x()));

    mapVolumeDataToNekVolume(e, _disp_y_var, 1.0 / _L_ref, &_displacement_y);
    writeVolumeSolution(e, field::y_displacement, _displacement_y, &(_nek_mesh->nek_initial_y()));

    mapVolumeDataToNekVolume(e, _disp_z_var, 1.0 / _L_ref, &_displacement_z);
    writeVolumeSolution(e, field::z_displacement, _displacement_z, &(_nek_mesh->nek_initial_z()));
  }

  _displaced_problem->updateMesh();
}

bool
NekRSProblem::normalizeHeatSource(const double moose, double nek, double & normalized_nek)
{
  auto dimension_multiplier = nekrs::referenceVolume() * nekrs::referenceSource();

  // scale the nek source to dimensional form for the sake of normalizing against
  // a dimensional MOOSE source
  nek *= dimension_multiplier;

  // avoid divide-by-zero
  if (std::abs(nek) < _abs_tol)
    return true;

  nekrs::scaleUsrwrk(indices.heat_source, moose / nek);

  // check that the normalization worked properly
  normalized_nek =
      nekrs::usrwrkVolumeIntegral(indices.heat_source, nek_mesh::all) * dimension_multiplier;
  bool low_rel_err = std::abs(normalized_nek - moose) / moose < _rel_tol;
  bool low_abs_err = std::abs(normalized_nek - moose) < _abs_tol;

  return low_rel_err && low_abs_err;
}

void
NekRSProblem::sendVolumeHeatSourceToNek()
{
  _console << "Sending volumetric heat source to NekRS" << std::endl;

  for (unsigned int e = 0; e < _n_volume_elems; e++)
  {
    // We can only write into the nekRS scratch space if that face is "owned" by the current process
    if (nekrs::commRank() != _nek_mesh->volumeCoupling().processor_id(e))
      continue;

    mapVolumeDataToNekVolume(e, _heat_source_var, 1.0 / nekrs::referenceSource(), &_source_elem);
    writeVolumeSolution(e, field::heat_source, _source_elem);
  }

  // Because the NekRSMesh may be quite different from that used in the app solving for
  // the heat source, we will need to normalize the total source on the nekRS side by the
  // total source computed by the coupled MOOSE app.
  const Real scale_cubed = _nek_mesh->scaling() * _nek_mesh->scaling() * _nek_mesh->scaling();
  const double nek_source = nekrs::usrwrkVolumeIntegral(indices.heat_source, nek_mesh::all);
  const double moose_source = *_source_integral;

  // For the sake of printing diagnostics to the screen regarding source normalization,
  // we first scale the nek source by any unit changes and then by the reference source
  const double nek_source_print_mult = scale_cubed * nekrs::referenceSource();
  double normalized_nek_source = 0.0;
  bool successful_normalization;

  _console << "Normalizing total NekRS heat source of "
           << Moose::stringify(nek_source * nek_source_print_mult)
           << " to the conserved MOOSE value of " + Moose::stringify(moose_source) << std::endl;

  // Any unit changes (for DIMENSIONAL nekRS runs) are automatically accounted for
  // here because moose_source is an integral on the MOOSE mesh, while nek_source is
  // an integral on the nek mesh
  successful_normalization = normalizeHeatSource(moose_source, nek_source, normalized_nek_source);

  // If before normalization, there is a large difference between the nekRS imposed source
  // and the MOOSE source, this could mean that there is a poor match between the domains,
  // even if neither value is zero. For instance, if you forgot that the nekRS mesh is in
  // units of centimeters, but you're coupling to an app based in meters, the sources will
  // be very different from one another.
  if (moose_source &&
      (std::abs(nek_source * nek_source_print_mult - moose_source) / moose_source) > 0.25)
    mooseDoOnce(mooseWarning("nekRS source differs from MOOSE source by more than 25\%! "
                             "This could indicate that your geometries do not line up properly "
                             "or there is some other mistake in the data transfer (check the "
                             "exodus output files to see if the transferred data looks sensible)."));

  if (!successful_normalization)
    mooseError("Heat source normalization process failed! nekRS integrated heat source: ",
               normalized_nek_source,
               " MOOSE integrated heat source: ",
               moose_source,
               ".\n\nThis may happen if the nekRS mesh "
               "is very different from that used in the App sending heat source to nekRS and the "
               "nearest node transfer is only picking up zero values in the coupled App."
               "OR, this error could indicate that your tolerances for comparing the re-normalized "
               "Nek heat source with the incoming MOOSE heat source are too tight. If the NekRS heat source (",
               normalized_nek_source, ") is acceptably close to the MOOSE heat source (", moose_source, "), "
               "then you can try relaxing the 'normalization_abs_tol' and/or 'normalization_rel_tol' "
               "parameters");
}

void
NekRSProblem::getBoundaryTemperatureFromNek()
{
  _console << "Extracting NekRS temperature from boundary " << Moose::stringify(*_boundary)
           << std::endl;

  // Get the temperature solution from nekRS. Note that nekRS performs a global communication
  // here such that each nekRS process has all the boundary temperature information. That is,
  // every process knows the full boundary temperature solution
  boundarySolution(field::temperature, _T);
}

void
NekRSProblem::getVolumeTemperatureFromNek()
{
  _console << "Extracting NekRS temperature from volume" << std::endl;

  // Get the temperature solution from nekRS. Note that nekRS performs a global communication
  // here such that each nekRS process has all the volume temperature information. In
  // other words, regardless of which elements a nek rank owns, after calling nekrs::temperature,
  // every process knows the temperature in the volume.
  volumeSolution(field::temperature, _T);
}

void
NekRSProblem::syncSolutions(ExternalProblem::Direction direction)
{
  auto & solution = _aux->solution();

  if (!isDataTransferHappening(direction))
    return;

  switch (direction)
  {
    case ExternalProblem::Direction::TO_EXTERNAL_APP:
    {
      if (_first)
      {
        _serialized_solution->init(_aux->sys().n_dofs(), false, SERIAL);
        _first = false;
      }

      solution.localize(*_serialized_solution);

      if (_boundary)
        sendBoundaryHeatFluxToNek();

      if (_volume && _has_heat_source)
        sendVolumeHeatSourceToNek();

      if (nekrs::hasUserMeshSolver())
        sendVolumeDeformationToNek();
      else if (nekrs::hasBlendingSolver())
        sendBoundaryDeformationToNek();

      sendScalarValuesToNek();

      copyScratchToDevice();

      break;
    }

    case ExternalProblem::Direction::FROM_EXTERNAL_APP:
    {
      if (!_volume)
        getBoundaryTemperatureFromNek();

      if (_volume)
        getVolumeTemperatureFromNek();

      // for boundary-only coupling, this fills a variable on a boundary mesh; otherwise,
      // this fills a variable on a volume mesh (because we will want a volume temperature for
      // neutronics feedback, and we can still get a temperature boundary condition from a volume
      // set)
      fillAuxVariable(_temp_var, _T);

      _console << " Interpolated temperature min/max values: " << minInterpolatedTemperature()
               << ", " << maxInterpolatedTemperature() << std::endl;

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

  solution.close();
  _aux->system().update();
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

  checkDuplicateVariableName("temp");
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
    checkDuplicateVariableName("avg_flux");
    addAuxVariable("MooseVariable", "avg_flux", var_params);
    _avg_flux_var = _aux->getFieldVariable<Real>(0, "avg_flux").number();

    // add the postprocessor that receives the flux integral for normalization
    if (_conserve_flux_by_sideset)
    {
      auto vpp_params = _factory.getValidParams("ConstantVectorPostprocessor");

      // create zero initial values
      std::vector<std::vector<Real>> dummy_vals(1, std::vector<Real>(_boundary->size()));
      vpp_params.set<std::vector<std::vector<Real>>>("value") = dummy_vals;
      addVectorPostprocessor("ConstantVectorPostprocessor", "flux_integral", vpp_params);
    }
    else
    {
      auto pp_params = _factory.getValidParams("Receiver");
      addPostprocessor("Receiver", "flux_integral", pp_params);
    }
  }

  if (_volume && _has_heat_source)
  {
    checkDuplicateVariableName("heat_source");
    addAuxVariable("MooseVariable", "heat_source", var_params);
    _heat_source_var = _aux->getFieldVariable<Real>(0, "heat_source").number();

    // add the postprocessor that receives the source integral for normalization
    auto pp_params = _factory.getValidParams("Receiver");
    addPostprocessor("Receiver", "source_integral", pp_params);
  }

  // add the displacement aux variables from the solid mechanics solver; these will
  // be needed regardless of whether the displacement is boundary- or volume-based
  if (nekrs::hasMovingMesh())
  {
    checkDuplicateVariableName("disp_x");
    addAuxVariable("MooseVariable", "disp_x", var_params);
    _disp_x_var = _aux->getFieldVariable<Real>(0, "disp_x").number();

    checkDuplicateVariableName("disp_y");
    addAuxVariable("MooseVariable", "disp_y", var_params);
    _disp_y_var = _aux->getFieldVariable<Real>(0, "disp_y").number();

    checkDuplicateVariableName("disp_z");
    addAuxVariable("MooseVariable", "disp_z", var_params);
    _disp_z_var = _aux->getFieldVariable<Real>(0, "disp_z").number();
  }
}

void
NekRSProblem::calculateMeshVelocity(int e, const field::NekWriteEnum & field)
{
  int len = _volume? _n_vertices_per_volume : _n_vertices_per_surface;
  double dt = _timestepper->getCurrentDT();

  double * displacement = nullptr, *prev_disp = nullptr;
  field::NekWriteEnum disp_field;

  switch (field)
  {
    case field::mesh_velocity_x:
      displacement = _displacement_x;
      prev_disp = _nek_mesh->prev_disp_x().data();
      disp_field = field::x_displacement;
      break;
    case field::mesh_velocity_y:
      displacement = _displacement_y;
      prev_disp = _nek_mesh->prev_disp_y().data();
      disp_field = field::y_displacement;
      break;
    case field::mesh_velocity_z:
      displacement = _displacement_z;
      prev_disp = _nek_mesh->prev_disp_z().data();
      disp_field = field::z_displacement;
      break;
    default:
      mooseError("Unhandled NekWriteEnum in NekRSProblem::calculateMeshVelocity!\n");
  }

  for (int i=0; i <len; i++)
    _mesh_velocity_elem[i] = (displacement[i] - prev_disp[(e*len) + i])/dt/_U_ref;

  _nek_mesh->updateDisplacement(e, displacement, disp_field);
}
#endif
