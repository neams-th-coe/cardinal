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

#include "NekVolumetricSource.h"

registerMooseObject("CardinalApp", NekVolumetricSource);

InputParameters
NekVolumetricSource::validParams()
{
  auto params = ConservativeFieldTransfer::validParams();
  params.addParam<Real>(
      "initial_source_integral",
      0,
      "Initial value to use for the 'postprocessor_to_conserve'; this initial value will be "
      "overridden once the coupled app executes its transfer of the volumetric source term "
      "integral into the 'postprocessor_to_conserve'. You may want to use this parameter if NekRS "
      "runs first, or if you are running NekRS in isolation but still want to apply a source term "
      "via Cardinal. Remember that this parameter is only used to normalize the source term "
      "'source_variable', so you will need to populate an initial shape.");
  params.addClassDescription("Reads/writes volumetric source data between NekRS and MOOSE.");
  return params;
}

NekVolumetricSource::NekVolumetricSource(const InputParameters & parameters)
  : ConservativeFieldTransfer(parameters),
    _initial_source_integral(getParam<Real>("initial_source_integral"))
{
  if (_direction == "to_nek")
  {
    addExternalVariable(_usrwrk_slot[0], _variable);
    indices.heat_source = _usrwrk_slot[0] * nekrs::fieldOffset();

    if (_usrwrk_slot.size() > 1)
      paramError("usrwrk_slot",
                 "'usrwrk_slot' must be of length 1 for volumetric source transfers; you have "
                 "entered a vector of length " +
                     Moose::stringify(_usrwrk_slot.size()));
  }

  if (!_nek_mesh->volume())
    mooseError("The NekVolumetricSource object can only be used when there is volumetric coupling "
               "of NekRS with MOOSE, i.e. when 'volume = true' in NekRSMesh.");

  if (_direction == "from_nek")
    paramError("direction",
               "The NekVolumetricSource currently only supports transfers 'to_nek'; contact the "
               "Cardinal developer team if you require reading of NekRS volumetric source terms.");

  // Check that there is a udf function providing the source for the passive scalar
  // equations. NOTE: This check is imperfect, because even if there is a source kernel,
  // we cannot tell _which_ passive scalar equation that it is applied to (we have
  // source kernels for the RANS passive scalar equations, for instance).
  if (nekrs::hasTemperatureSolve() && !nekrs::hasHeatSourceKernel())
    mooseError("In order to send a volumetric heat source to NekRS, you must have an OCCA source "
               "kernel in the passive scalar equations!");

  if (!nekrs::hasTemperatureVariable())
    mooseError("In order to send a volumetric heat source to NekRS, your case files must have a "
               "[TEMPERATURE] block. Note that you can set 'solver = none' in '" +
               _nek_problem.casename() + ".par' if you don't want to solve for temperature.");

  if (!nekrs::hasTemperatureSolve())
    mooseWarning("By setting 'solver = none' for temperature in '" + _nek_problem.casename() +
                 ".par', NekRS will not solve for temperature. The volumetric heat source sent by "
                 "this object will be unused.");

  addExternalPostprocessor(_postprocessor_name, _initial_source_integral);
  _source_integral = &getPostprocessorValueByName(_postprocessor_name);
}

bool
NekVolumetricSource::normalizeVolumetricSource(const double moose,
                                               double nek,
                                               double & normalized_nek)
{
  auto dimension_multiplier =
      nekrs::referenceVolume() * nekrs::nondimensionalDivisor(field::heat_source);

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
NekVolumetricSource::sendDataToNek()
{
  _console << "Sending volumetric source to NekRS..." << std::endl;

  auto d = nekrs::nondimensionalDivisor(field::heat_source);
  auto a = nekrs::nondimensionalAdditive(field::heat_source);
  for (unsigned int e = 0; e < _nek_mesh->numVolumeElems(); e++)
  {
    // We can only write into the nekRS scratch space if that face is "owned" by the current process
    if (nekrs::commRank() != _nek_mesh->volumeCoupling().processor_id(e))
      continue;

    _nek_problem.mapVolumeDataToNekVolume(e, _variable_number[_variable], d, a, &_v_elem);
    _nek_problem.writeVolumeSolution(e, field::heat_source, _v_elem);
  }

  // Because the NekRSMesh may be quite different from that used in the app solving for
  // the heat source, we will need to normalize the total source on the nekRS side by the
  // total source computed by the coupled MOOSE app.
  const Real scale_cubed = _nek_mesh->scaling() * _nek_mesh->scaling() * _nek_mesh->scaling();
  const double nek_source = nekrs::usrwrkVolumeIntegral(indices.heat_source, nek_mesh::all);
  const double moose_source = *_source_integral;

  // For the sake of printing diagnostics to the screen regarding source normalization,
  // we first scale the nek source by any unit changes and then by the reference source
  const double nek_source_print_mult =
      scale_cubed * nekrs::nondimensionalDivisor(field::heat_source);
  double normalized_nek_source = 0.0;
  bool successful_normalization;

  _console << "[volume]: Normalizing total NekRS heat source of "
           << Moose::stringify(nek_source * nek_source_print_mult)
           << " to the conserved MOOSE value of " + Moose::stringify(moose_source) << std::endl;

  // Any unit changes (for DIMENSIONAL nekRS runs) are automatically accounted for
  // here because moose_source is an integral on the MOOSE mesh, while nek_source is
  // an integral on the nek mesh
  successful_normalization =
      normalizeVolumetricSource(moose_source, nek_source, normalized_nek_source);

  // If before normalization, there is a large difference between the nekRS imposed source
  // and the MOOSE source, this could mean that there is a poor match between the domains,
  // even if neither value is zero. For instance, if you forgot that the nekRS mesh is in
  // units of centimeters, but you're coupling to an app based in meters, the sources will
  // be very different from one another.
  if (moose_source &&
      (std::abs(nek_source * nek_source_print_mult - moose_source) / moose_source) > 0.25)
    mooseDoOnce(
        mooseWarning("NekRS source differs from MOOSE source by more than 25\%! This is NOT "
                     "necessarily a problem - but it could indicate that your geometries don't "
                     "line up properly or something is amiss with your transfer. We recommend "
                     "opening the output files to visually inspect the volumetric source in both "
                     "the main and sub applications to check that the fields look correct."));

  if (!successful_normalization)
    mooseError("Volumetric source normalization process failed! NekRS integrated source: ",
               normalized_nek_source,
               " MOOSE integrated source: ",
               moose_source,
               ".\n\n",
               normalizationHint());
}

#endif
