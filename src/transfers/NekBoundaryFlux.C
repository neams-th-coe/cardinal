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

#include "NekBoundaryFlux.h"
#include "AuxiliarySystem.h"

registerMooseObject("CardinalApp", NekBoundaryFlux);

extern nekrs::usrwrkIndices indices;

InputParameters
NekBoundaryFlux::validParams()
{
  // TODO: class needs to be generalized to any volumetric source; currently only
  // written for heat sources
  auto params = FieldTransferBase::validParams();
  params.addParam<std::string>("postprocessor_to_conserve", "Name of the postprocessor containing the integral of the flux term in order to ensure conservation; defaults to the name of the object plus '_integral'");
  params.addParam<unsigned int>("usrwrk_slot", "When 'direction = to_nek', the slot in the usrwrk array to write the flux term.");
  params.addParam<Real>(
    "initial_flux_integral",
    0,
    "Initial value to use for the 'postprocessor_to_conserve', to ensure conservation; this initial value will be overridden once the coupled app executes its transfer of the boundary flux term integral into the 'postprocessor_to_conserve'. You may want to use this parameter if NekRS runs first, or if you are running NekRS in isolation but still want to apply a boundary flux term via Cardinal. Remember that this parameter is only used to normalize the flux, so you will need to populate an initial shape (magnitude is unimportant because it will be normalized by this parameter).");

  params.addRangeCheckedParam<Real>("normalization_abs_tol", 1e-8, "normalization_abs_tol > 0",
    "Absolute tolerance for checking if conservation is maintained during transfer");

  params.addRangeCheckedParam<Real>("normalization_rel_tol", 1e-5, "normalization_rel_tol > 0",
    "Relative tolerance for checking if conservation is maintained during transfer");
  params.addParam<bool>("conserve_flux_by_sideset", false,
    "Whether to conserve the flux by individual sideset (as opposed to lumping all sidesets together). Setting this option to true requires syntax changes in the input file to use vector postprocessors, and places restrictions on how the sidesets are set up.");

  params.addClassDescription("Reads/writes boundary flux data between NekRS and MOOSE.");
  return params;
}

NekBoundaryFlux::NekBoundaryFlux(const InputParameters & parameters)
  : FieldTransferBase(parameters),
    PostprocessorInterface(this),
    _conserve_flux_by_sideset(getParam<bool>("conserve_flux_by_sideset")),
    _initial_flux_integral(getParam<Real>("initial_flux_integral")),
    _boundary(_nek_mesh->boundary()),
    _abs_tol(getParam<Real>("normalization_abs_tol")),
    _rel_tol(getParam<Real>("normalization_rel_tol"))
{
  if (!_nek_problem.boundary())
    mooseError("The NekBoundaryFlux object can only be used when there is boundary coupling of NekRS with MOOSE, i.e. when 'boundary' is provided in NekRSMesh.");

  if (_direction == "to_nek")
  {
    checkRequiredParam(parameters, "usrwrk_slot", "'direction = to_nek'");
    indices.flux = getParam<unsigned int>("usrwrk_slot") * nekrs::fieldOffset();
  }
  else if (_direction == "from_nek")
    paramError("direction", "The NekBoundaryFlux currently only supports transfers 'to_nek'; contact the Cardinal developer team if you require reading of NekRS boundary flux terms.");
  else
    mooseError("Unhandled NekDirectionEnum in NekBoundaryFlux!");

  std::string postprocessor_name;
  if (isParamValid("postprocessor_to_conserve"))
    postprocessor_name = getParam<std::string>("postprocessor_to_conserve");
  else
    postprocessor_name = name() + "_integral";

  // Check that the correct flux boundary condition is set on all of nekRS's
  // boundaries. To avoid throwing this error for test cases where we have a
  // [TEMPERATURE] block but set its solve to 'none', we also check whether
  // we're actually computing for the temperature.
  if (_boundary && nekrs::hasTemperatureSolve())
  {
    for (const auto & b : *_boundary)
      if (!nekrs::isHeatFluxBoundary(b))
      {
        const std::string type = nekrs::temperatureBoundaryType(b);
        mooseError("In order to send a boundary heat flux to nekRS, you must have a flux condition "
                   "for each 'boundary' set in 'NekRSMesh'!\nBoundary " +
                   std::to_string(b) + " is of type '" + type + "' instead of 'fixedGradient'.");
      }
  }

  // add the postprocessor that receives the flux integral for normalization
    if (_conserve_flux_by_sideset)
    {
      if (isParamSetByUser("initial_flux_integral"))
        mooseWarning("The 'initial_flux_integral' capability is not yet supported when "
                     "'conserve_flux_by_sideset' is enabled. Please contact a Cardinal developer "
                     "if this is hindering your use case.");

      auto vpp_params = _factory.getValidParams("ConstantVectorPostprocessor");

      // create zero initial values
      std::vector<std::vector<Real>> dummy_vals(1, std::vector<Real>(_boundary->size()));
      vpp_params.set<std::vector<std::vector<Real>>>("value") = dummy_vals;
      _nek_problem.addVectorPostprocessor("ConstantVectorPostprocessor", "flux_integral", vpp_params);
    }
    else
      addExternalPostprocessor(postprocessor_name, _initial_flux_integral);

  if (_conserve_flux_by_sideset)
    _flux_integral_vpp = &_nek_problem.getVectorPostprocessorValueByName(postprocessor_name, "value");
  else
    _flux_integral = &getPostprocessorValueByName(postprocessor_name);

  int n_per_surf = _nek_mesh->exactMirror() ?
    std::pow(_nek_mesh->nekNumQuadraturePoints1D(), 2.0) : _nek_mesh->numVerticesPerSurface();
  int n_per_vol = _nek_mesh->exactMirror() ?
    std::pow(_nek_mesh->nekNumQuadraturePoints1D(), 3.0) : _nek_mesh->numVerticesPerVolume();
  _flux_face = (double *)calloc(n_per_surf, sizeof(double));
  _flux_elem = (double *)calloc(n_per_vol, sizeof(double));
}

NekBoundaryFlux::~NekBoundaryFlux()
{
  freePointer(_flux_face);
  freePointer(_flux_elem);
}

void
NekBoundaryFlux::sendDataToNek()
{
  _console << "Sending heat flux to NekRS boundary " << Moose::stringify(*_boundary) << "..."
           << std::endl;

  if (!_nek_problem.volume())
  {
    for (unsigned int e = 0; e < _nek_mesh->numSurfaceElems(); e++)
    {
      // We can only write into the nekRS scratch space if that face is "owned" by the current process
      if (nekrs::commRank() != _nek_mesh->boundaryCoupling().processor_id(e))
        continue;

      _nek_problem.mapFaceDataToNekFace(e, _variable_number, 1.0 / nekrs::referenceFlux(), &_flux_face);
      _nek_problem.writeBoundarySolution(e, field::flux, _flux_face);
    }
  }
  else
  {
    for (unsigned int e = 0; e < _nek_mesh->numVolumeElems(); ++e)
    {
      // We can only write into the nekRS scratch space if that face is "owned" by the current process
      if (nekrs::commRank() != _nek_mesh->volumeCoupling().processor_id(e))
        continue;

      _nek_problem.mapFaceDataToNekVolume(e, _variable_number, 1.0 / nekrs::referenceFlux(), &_flux_elem);
      _nek_problem.writeVolumeSolution(e, field::flux, _flux_elem);
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
  else                                                                                       {
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
NekBoundaryFlux::checkInitialFluxValues(const Real & nek_flux, const Real & moose_flux) const
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
#endif
