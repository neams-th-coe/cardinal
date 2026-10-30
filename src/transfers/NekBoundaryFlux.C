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

registerMooseObject("CardinalApp", NekBoundaryFlux);

InputParameters
NekBoundaryFlux::validParams()
{
  auto params = ConservativeFieldTransfer::validParams();
  params.addParam<Real>(
      "initial_flux_integral",
      0,
      "Initial value to use for the 'postprocessor_to_conserve'; this initial value will be "
      "overridden once the coupled app executes its transfer of the boundary flux term integral "
      "into the 'postprocessor_to_conserve'. You may want to use this parameter if NekRS runs "
      "first, or if you are running NekRS in isolation but still want to apply a mirror boundary flux "
      "term via Cardinal. Remember that this parameter is only used to normalize the flux, so you "
      "will need to populate an initial shape (or constant value) to the auxvariable holding the "
      "flux if you want to see this parameter take effect.");

  // TODO: support initial_flux_integral_stl (check it is unused)
  params.addParam<Real>(
      "initial_flux_integral_stl",
      0,
      "Initial value to use for the 'postprocessor_to_conserve'; this initial value will be "
      "overridden once the coupled app executes its transfer of the boundary flux term integral "
      "into the 'postprocessor_to_conserve'. You may want to use this parameter if NekRS runs "
      "first, or if you are running NekRS in isolation but still want to apply an STL boundary flux "
      "term via Cardinal. Remember that this parameter is only used to normalize the flux, so you "
      "will need to populate an initial shape (or constant value) to the auxvariable holding the "
      "flux if you want to see this parameter take effect.");

  params.addParam<bool>(
      "conserve_flux_by_sideset",
      false,
      "Whether to conserve the flux by individual sideset (as opposed to lumping all sidesets "
      "together). Setting this option to true requires syntax changes in the input file to use "
      "vector postprocessors, and places restrictions on how the sidesets are set up.");

  params.addClassDescription("Reads/writes boundary flux data between NekRS and MOOSE.");
  return params;
}

NekBoundaryFlux::NekBoundaryFlux(const InputParameters & parameters)
  : ConservativeFieldTransfer(parameters),
    _conserve_flux_by_sideset(getParam<bool>("conserve_flux_by_sideset")),
    _initial_flux_integral(getParam<Real>("initial_flux_integral")),
    _initial_flux_integral_stl(getParam<Real>("initial_flux_integral_stl")),
    _boundary(_nek_mesh->boundary()),
    _reference_flux_integral(nekrs::referenceArea() * nekrs::nondimensionalDivisor(field::flux))
{
  if (_transfer_mesh.contains("mirror"))
  {
    if (!_boundary)
      mooseError("When transferring data with the mesh mirror, NekBoundaryFlux requires 'boundary' "
                 "to be provided in NekRSMesh.");

    // right now, all of our systems used for transferring data assume that if we have a volume
    // mesh mirror, that we will be writing data to that entire mesh mirror. But this is not the
    // case if we want to write a flux, which we would only write on a boundary associated with
    // a unit normal. For now, just prevent users from doing this.
    if (_direction == "from_nek" && _nek_mesh->volume())
      mooseError("The NekBoundaryFlux does not currently support writing heat flux on a boundary "
                 "when the Mesh has 'volume = true.' Please contact the Cardinal developer team if "
                 "you require this feature.");
  }
  else
    checkUnusedParam(parameters, "initial_flux_integral", "the 'mesh' does not contain 'mirror'");

  // TODO: test
  if (_transfer_mesh.contains("stl") && _transfer_mesh.contains("mirror"))
    mooseError("Cardinal does not yet support a single transfer being used to read flux from both an STL surface and the mesh mirror, because different-order variables are created for each. Please use two separate NekBoundaryFlux transfers, one for the mirro and the other for the STL.");

  if (!_transfer_mesh.contains("stl"))
    checkUnusedParam(parameters, "initial_flux_integral_stl", "the 'mesh' does not contain 'stl'");

  // TODO: will need to be generalized for other scalars
  if (!nekrs::hasTemperatureVariable())
    mooseError("In order to read or write NekRS's boundary heat flux, your case files must have a "
               "[TEMPERATURE] block. Note that you can set 'solver = none' in '" +
               _nek_problem.casename() + ".par' if you don't want to solve for temperature.");

  // add the variables for the coupling and perform checks on problem setup
  if (_direction == "from_nek")
  {
    if (_conserve_flux_by_sideset)
      paramError("conserve_flux_by_sideset",
                 "When 'direction = from_nek', the 'conserve_flux_by_sideset' option is not yet "
                 "supported. Contact the Cardinal developer team if you need this feature.");

    checkUnusedParam(parameters, "initial_flux_integral", "'direction = from_nek'");
    addExternalVariable(_variable);
  }
  else
  {
    if (_usrwrk_slot.size() > 1)
      paramError("usrwrk_slot",
                 "'usrwrk_slot' must be of length 1 for boundary flux transfers; you have entered "
                 "a vector of length " +
                     Moose::stringify(_usrwrk_slot.size()));

    // TODO: this will need to be generalized if the same transfer is used for fluxes of varying
    // interpretation
    auto d = nekrs::nondimensionalDivisor(field::flux);
    auto a = nekrs::nondimensionalAdditive(field::flux);

    // TODO: relax this constraint, it would be better to write sendSTLDataToNek so that it works
    // with linear Lagrange
    //if (_transfer_mesh.contains("mirror"))
      addExternalVariable(_usrwrk_slot[0], _variable, a, d);
    //else
    //  addExternalMonomialVariable(_usrwrk_slot[0], _variable, a, d);

    // Check that the correct flux boundary condition is set on all of nekRS's
    // boundaries. To avoid throwing this error for test cases where we have a
    // [TEMPERATURE] block but set its solve to 'none', we screen by whether the
    // temperature solve is enabled. This check only makes sense for the mirror mesh, because
    // otherwise the STL is not an actual sideset
    if (_transfer_mesh.contains("mirror"))
      if (_boundary && nekrs::hasTemperatureSolve())
        for (const auto & b : *_boundary)
          if (!nekrs::isHeatFluxBoundary(b))
            mooseError("In order to send a boundary heat flux to NekRS, you must have a heat flux "
                       "condition for each 'boundary' set in 'NekRSMesh'! Boundary " +
                       std::to_string(b) + " is of type '" + nekrs::temperatureBoundaryType(b) +
                       "' instead of 'udfNeumann'.");

    if (!nekrs::hasTemperatureSolve())
      mooseWarning("By setting 'solver = none' for temperature in '" + _nek_problem.casename() +
                   ".par', NekRS will not solve for temperature. The heat flux sent by this object "
                   "will be unused.");
  }

  // add postprocessors to hold integrated heat flux; we do not need to check for duplicate
  // names, because MOOSE already handles this error checking
  if (_direction == "to_nek")
  {
    // add the postprocessor that receives the flux integral for normalization
    if (_conserve_flux_by_sideset)
    {
      if (_transfer_mesh.contains("stl"))
        paramError("conserve_flux_by_sideset", "This option is not yet supported for STL data transfers; please contact the Cardinal developer team if you need this feature");

      if (isParamSetByUser("initial_flux_integral"))
        mooseWarning("The 'initial_flux_integral' capability is not yet supported when "
                     "'conserve_flux_by_sideset' is enabled. Please contact a Cardinal developer "
                     "if this is hindering your use case.");

      auto vpp_params = _factory.getValidParams("ConstantVectorPostprocessor");

      // create zero initial values
      std::vector<std::vector<Real>> dummy_vals(1, std::vector<Real>(_boundary->size()));
      vpp_params.set<std::vector<std::vector<Real>>>("value") = dummy_vals;
      _nek_problem.addVectorPostprocessor(
          "ConstantVectorPostprocessor", _postprocessor_name, vpp_params);
      _flux_integral_vpp =
          &_nek_problem.getVectorPostprocessorValueByName(_postprocessor_name, "value");
    }
    else
    {
      if (_transfer_mesh.contains("mirror"))
      {
        addExternalPostprocessor(_postprocessor_name, _initial_flux_integral);
        _flux_integral = &getPostprocessorValueByName(_postprocessor_name);
      }
      if (_transfer_mesh.contains("stl"))
      {
        addExternalPostprocessor(_postprocessor_name_stl, _initial_flux_integral_stl);
        _flux_integral_stl = &getPostprocessorValueByName(_postprocessor_name_stl);
      }
    }
  }
  else
  {
    if (_transfer_mesh.contains("mirror"))
    {
      // create a NekHeatFluxIntegral postprocessor to compute the heat flux; this represents
      // the actual heat flux computed by NekRS
      auto pp_params = _factory.getValidParams("NekHeatFluxIntegral");
      pp_params.set<std::vector<int>>("boundary") = *_boundary;
      _nek_problem.addPostprocessor("NekHeatFluxIntegral", _postprocessor_name, pp_params);
      _flux_integral = &getPostprocessorValueByName(_postprocessor_name);
    }

    if (_transfer_mesh.contains("stl"))
    {
      // create a surface integral of the evaluated heat flux on the STL surface; note that
      // the STL geometry is a volume (2-D), so that a volume integral is equivalent to a
      // surface integral. The STL surface is not a physical sideset in the NekRS problem,
      // so this postprocessor is only an approximation to the heat flux on this surface.
      // TODO: check whether we need a different PP here
      auto pp_params = _factory.getValidParams("ElementIntegralVariablePostprocessor");
      pp_params.set<std::vector<SubdomainName>>("block").push_back(
          Moose::stringify(_nek_mesh->STLSubdomainID()));
      pp_params.set<std::vector<VariableName>>("variable").push_back(_variable);
      _nek_problem.addPostprocessor(
          "ElementIntegralVariablePostprocessor", _postprocessor_name_stl, pp_params);
      _flux_integral_stl = &getPostprocessorValueByName(_postprocessor_name_stl);
    }
  }
}

void
NekBoundaryFlux::readDataFromNek()
{
  if (_transfer_mesh.contains("mirror"))
  {
    if (!_nek_mesh->volume())
      _nek_problem.boundarySolution(field::flux, _external_data);
    else
      _nek_problem.volumeSolution(field::flux, _external_data);

    fillAuxVariableOnMirror(_variable_number[_variable], _external_data);
  }

  if (_transfer_mesh.contains("stl"))
  {
    auto interpolated = nekrs::interpolateHeatFluxToPoints(
        *_interpolator, _nek_mesh->STLNormals(), _nek_mesh->nSTLVertices());
    fillAuxVariableOnSTL(_variable_number[_variable], interpolated.data());
  }

  // the postprocessor(s) are automatically populated because its execution controls its writing
}

void
NekBoundaryFlux::sendDataToNek()
{
  auto d = nekrs::nondimensionalDivisor(field::flux);
  auto a = nekrs::nondimensionalAdditive(field::flux);

  // Because the NekRSMesh may be quite different from that used in the app solving for
  // the heat flux, we will need to normalize the flux on the nekRS side by the
  // flux computed by the coupled MOOSE app. For this and the next check of the
  // flux integral, we need to scale the integral back up again to the dimensional form
  // for the sake of comparison.
  const Real scale_squared = _nek_mesh->scaling() * _nek_mesh->scaling();
  const double nek_flux_print_mult = scale_squared * nekrs::nondimensionalDivisor(field::flux);
  double normalized_nek_flux = 0.0;
  double total_moose_flux;
  bool successful_normalization = false;

  if (_transfer_mesh.contains("mirror"))
  {
    _console << "Sending flux to NekRS boundary " << Moose::stringify(*_boundary) << "..."
             << std::endl;
    sendFaceDataToNek(_variable_number[_variable], d, a);

    // integrate the flux over each individual boundary
    std::vector<double> nek_flux_sidesets =
        nekrs::usrwrkSideIntegral(_usrwrk_slot[0] * nekrs::fieldOffset(), *_boundary, nek_mesh::all);

    if (_conserve_flux_by_sideset)
    {
      auto moose_flux = *_flux_integral_vpp;
      if (moose_flux.size() != _boundary->size())
        mooseError("The sideset flux reporter transferred to NekRS must have a length equal to the "
                   "number of entries in 'boundary'! Please check the values written to the "
                   "'flux_integral' vector postprocessor.\n\n"
                   "Length of reporter: ",
                   moose_flux.size(),
                   "\n",
                   "Length of 'boundary': ",
                   _boundary->size());

      for (std::size_t b = 0; b < _boundary->size(); ++b)
      {
        _console << "[boundary " << Moose::stringify((*_boundary)[b])
                 << "]: Normalizing NekRS flux of "
                 << Moose::stringify(nek_flux_sidesets[b] * nek_flux_print_mult)
                 << " to the conserved MOOSE value of " << Moose::stringify(moose_flux[b])
                 << std::endl;

        checkInitialFluxValues(nek_flux_sidesets[b], moose_flux[b]);
      }

      total_moose_flux = std::accumulate(moose_flux.begin(), moose_flux.end(), 0.0);

      // For the sake of printing diagnostics to the screen regarding the flux normalization,
      // we first scale the nek flux by any unit changes and then by the reference flux.
      successful_normalization =
          normalizeFluxBySideset(moose_flux, nek_flux_sidesets, normalized_nek_flux);
    }
    else
    {
      auto moose_flux = *_flux_integral;
      const double nek_flux =
          std::accumulate(nek_flux_sidesets.begin(), nek_flux_sidesets.end(), 0.0);

      _console << "[boundary " << Moose::stringify(*_boundary)
               << "]: Normalizing total NekRS flux of "
               << Moose::stringify(nek_flux * nek_flux_print_mult)
               << " to the conserved MOOSE value of " << Moose::stringify(moose_flux) << std::endl;
      _console << Moose::stringify(nek_flux) << " " << nek_flux_print_mult << std::endl;

      checkInitialFluxValues(nek_flux, moose_flux);

      total_moose_flux = moose_flux;

      // For the sake of printing diagnostics to the screen regarding the flux normalization,
      successful_normalization = normalizeFlux(moose_flux, nek_flux, normalized_nek_flux);
    }
  }

  if (_transfer_mesh.contains("stl"))
  {
    _console << "Sending flux from '" << _nek_mesh->STLFilename() << "' to NekRS..." << std::endl;
    std::cout << "variable number: " << _variable_number[_variable] << std::endl;
    sendSTLDataToNek(_variable_number[_variable]);

    // integrate the flux received onto the STL surface
    double nek_flux = nekrs::usrwrkSTLIntegral(_usrwrk_slot[0], _nek_mesh->STLVertices(), _nek_mesh->numSTLElems());

    auto moose_flux = *_flux_integral_stl;
    std::filesystem::path p(_nek_mesh->STLFilename());
    _console << "[" << p.filename().string()
             << "]: Normalizing total NekRS flux of "
             << Moose::stringify(nek_flux * nek_flux_print_mult)
             << " to the conserved MOOSE value of " << Moose::stringify(moose_flux) << std::endl;
  }

 // if (!successful_normalization)
 //   mooseError(
 //       "Flux normalization process failed! NekRS integrated flux: ",
 //       normalized_nek_flux,
 //       " MOOSE integrated flux: ",
 //       total_moose_flux,
 //       normalizationHint(),
 //       "\n\n- If you set 'conserve_flux_by_sideset = true' and nodes are SHARED by boundaries "
 //       "(like on corners between sidesets), you will end up renormalizing those shared nodes "
 //       "once per sideset that they lie on. There is no guarantee that the total imposed flux "
 //       "would be preserved.");
}

void
NekBoundaryFlux::checkInitialFluxValues(const Real & nek_flux, const Real & moose_flux) const
{
  const Real scale_squared = _nek_mesh->scaling() * _nek_mesh->scaling();
  const double nek_flux_print_mult = scale_squared * nekrs::nondimensionalDivisor(field::flux);

  // If before normalization, there is a large difference between the nekRS imposed flux
  // and the MOOSE flux, this could mean that there is a poor match between the domains,
  // even if neither value is zero. For instance, if you forgot that the nekRS mesh is in
  // units of centimeters, but you're coupling to an app based in meters, the fluxes will
  // be very different from one another.
  if (moose_flux && (std::abs(nek_flux * nek_flux_print_mult - moose_flux) / moose_flux) > 0.25)
    mooseDoOnce(mooseWarning(
        "NekRS flux differs from MOOSE flux by more than 25\%! This is NOT necessarily a problem - "
        "but it could indicate that your geometries don't line up properly or something is amiss "
        "with your transfer. We recommend opening the output files to visually inspect the flux in "
        "both the main and sub applications to check that the fields look correct."));
}

bool
NekBoundaryFlux::normalizeFluxBySideset(const std::vector<double> & moose_integral,
                                        std::vector<double> & nek_integral,
                                        double & normalized_nek_integral)
{
  // scale the nek flux to dimensional form for the sake of normalizing against
  // a dimensional MOOSE flux
  for (auto & i : nek_integral)
    i *= _reference_flux_integral;

  mesh_t * mesh = nekrs::temperatureMesh();
  auto nek_boundary_coupling = _nek_mesh->boundaryCoupling();

  auto usrwrk = nekrs::host_wrk();

  for (int k = 0; k < nek_boundary_coupling.total_n_faces; ++k)
  {
    if (nek_boundary_coupling.process[k] == nekrs::commRank())
    {
      int i = nek_boundary_coupling.element[k];
      int j = nek_boundary_coupling.face[k];

      int face_id = mesh->EToB[i * mesh->Nfaces + j];
      auto it = std::find(_boundary->begin(), _boundary->end(), face_id);
      auto b_index = it - _boundary->begin();

      // avoid divide-by-zero
      double ratio = 1.0;
      if (std::abs(nek_integral[b_index]) > _abs_tol)
        ratio = moose_integral[b_index] / nek_integral[b_index];

      int offset = i * mesh->Nfaces * mesh->Nfp + j * mesh->Nfp;

      for (int v = 0; v < mesh->Nfp; ++v)
      {
        int id = mesh->vmapM[offset + v];
        usrwrk[_usrwrk_slot[0] * nekrs::fieldOffset() + id] *= ratio;
      }
    }
  }

  // check that the normalization worked properly - confirm against dimensional form
  auto integrals =
      nekrs::usrwrkSideIntegral(_usrwrk_slot[0] * nekrs::fieldOffset(), *_boundary, nek_mesh::all);
  normalized_nek_integral =
      std::accumulate(integrals.begin(), integrals.end(), 0.0) * _reference_flux_integral;
  double total_moose_integral = std::accumulate(moose_integral.begin(), moose_integral.end(), 0.0);
  bool low_rel_err =
      std::abs(total_moose_integral) > _abs_tol
          ? std::abs(normalized_nek_integral - total_moose_integral) / total_moose_integral <
                _rel_tol
          : true;
  bool low_abs_err = std::abs(normalized_nek_integral - total_moose_integral) < _abs_tol;

  return low_rel_err || low_abs_err;
}

bool
NekBoundaryFlux::normalizeFlux(const double moose_integral,
                               double nek_integral,
                               double & normalized_nek_integral)
{
  // scale the nek flux to dimensional form for the sake of normalizing against
  // a dimensional MOOSE flux
  nek_integral *= _reference_flux_integral;

  auto nek_boundary_coupling = _nek_mesh->boundaryCoupling();

  // avoid divide-by-zero
  if (std::abs(nek_integral) < _abs_tol)
    return true;

  mesh_t * mesh = nekrs::temperatureMesh();

  const double ratio = moose_integral / nek_integral;

  auto usrwrk = nekrs::host_wrk();

  for (int k = 0; k < nek_boundary_coupling.total_n_faces; ++k)
  {
    if (nek_boundary_coupling.process[k] == nekrs::commRank())
    {
      int i = nek_boundary_coupling.element[k];
      int j = nek_boundary_coupling.face[k];
      int offset = i * mesh->Nfaces * mesh->Nfp + j * mesh->Nfp;

      for (int v = 0; v < mesh->Nfp; ++v)
      {
        int id = mesh->vmapM[offset + v];
        usrwrk[_usrwrk_slot[0] * nekrs::fieldOffset() + id] *= ratio;
      }
    }
  }

  // check that the normalization worked properly - confirm against dimensional form
  auto integrals =
      nekrs::usrwrkSideIntegral(_usrwrk_slot[0] * nekrs::fieldOffset(), *_boundary, nek_mesh::all);
  normalized_nek_integral =
      std::accumulate(integrals.begin(), integrals.end(), 0.0) * _reference_flux_integral;
  bool low_rel_err = std::abs(normalized_nek_integral - moose_integral) / moose_integral < _rel_tol;
  bool low_abs_err = std::abs(normalized_nek_integral - moose_integral) < _abs_tol;

  return low_rel_err || low_abs_err;
}

#endif
