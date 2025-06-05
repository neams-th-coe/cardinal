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
#include "AuxiliarySystem.h"

registerMooseObject("CardinalApp", NekVolumetricSource);

extern nekrs::usrwrkIndices indices;

InputParameters
NekVolumetricSource::validParams()
{
  // TODO: class needs to be generalized to any volumetric source; currently only
  // written for heat sources
  auto params = FieldTransferBase::validParams();
  params.addParam<std::string>("postprocessor_to_conserve", "Name of the postprocessor containing the integral of the source term in order to ensure conservation; defaults to the name of the object plus '_integral'");
  params.addParam<unsigned int>("usrwrk_slot", "When 'direction = to_nek', the slot in the usrwrk array to write the source term.");
  params.addParam<Real>(
    "initial_source_integral",
    0,
    "Initial value to use for the 'postprocessor_to_conserve', to ensure conservation; this initial value will be overridden once the coupled app executes its transfer of the volumetric source term integral into the 'postprocessor_to_conserve'. You may want to use this parameter if NekRS runs first, or if you are running NekRS in isolation but still want to apply a source term via Cardinal. Remember that this parameter is only used to normalize the source term 'source_variable', so you will need to populate an initial shape (magnitude is unimportant because it will be normalized by this parameter).");

  params.addRangeCheckedParam<Real>("normalization_abs_tol", 1e-8, "normalization_abs_tol > 0",
    "Absolute tolerance for checking if conservation is maintained during transfer");

  params.addRangeCheckedParam<Real>("normalization_rel_tol", 1e-5, "normalization_rel_tol > 0",
    "Relative tolerance for checking if conservation is maintained during transfer");

  params.addClassDescription("Reads/writes volumetric source data between NekRS and MOOSE.");
  return params;
}

NekVolumetricSource::NekVolumetricSource(const InputParameters & parameters)
  : FieldTransferBase(parameters),
    PostprocessorInterface(this),
    _initial_source_integral(getParam<Real>("initial_source_integral")),
    _abs_tol(getParam<Real>("normalization_abs_tol")),
    _rel_tol(getParam<Real>("normalization_rel_tol"))
{
  if (!_nek_problem.volume())
    mooseError("The NekVolumetricSource object can only be used when there is volumetric coupling of NekRS with MOOSE, i.e. when 'volume = true' in NekRSMesh.");

  if (_direction == "to_nek")
  {
    checkRequiredParam(parameters, "usrwrk_slot", "'direction = to_nek'");
    indices.heat_source = getParam<unsigned int>("usrwrk_slot") * nekrs::fieldOffset();
  }
  else if (_direction == "from_nek")
    paramError("direction", "The NekVolumetricSource currently only supports transfers 'to_nek'; contact the Cardinal developer team if you require reading of NekRS volumetric source terms.");
  else
    mooseError("Unhandled NekDirectionEnum in NekVolumetricSource!");

  std::string postprocessor_name;
  if (isParamValid("postprocessor_to_conserve"))
    postprocessor_name = getParam<std::string>("postprocessor_to_conserve");
  else
    postprocessor_name = name() + "_integral";

  // Check that there is a udf function providing the source for the passive scalar
  // equations. NOTE: This check is imperfect, because even if there is a source kernel,
  // we cannot tell _which_ passive scalar equation that it is applied to (we have
  // source kernels for the RANS passive scalar equations, for instance).
  if (nekrs::hasTemperatureSolve() && !nekrs::hasHeatSourceKernel())
    mooseError("In order to send a heat source to nekRS, you must have an OCCA kernel for the source in the passive scalar equations!");

  addExternalPostprocessor(postprocessor_name, _initial_source_integral);
  _source_integral = &getPostprocessorValueByName(postprocessor_name);

  int n_per_vol = _nek_mesh->exactMirror() ?
    std::pow(_nek_mesh->nekNumQuadraturePoints1D(), 3.0) : _nek_mesh->numVerticesPerVolume();
  _source_elem = (double *)calloc(n_per_vol, sizeof(double));
}

NekVolumetricSource::~NekVolumetricSource()
{
  freePointer(_source_elem);
}

bool
NekVolumetricSource::normalizeVolumetricSource(const double moose, double nek, double & normalized_nek)
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
NekVolumetricSource::sendDataToNek()
{
  _console << "Sending volumetric source to NekRS..." << std::endl;

  for (unsigned int e = 0; e < _nek_problem.nVolumeElems(); e++)
  {
    // We can only write into the nekRS scratch space if that face is "owned" by the current process
    if (nekrs::commRank() != _nek_mesh->volumeCoupling().processor_id(e))
      continue;

    _nek_problem.mapVolumeDataToNekVolume(e, _variable_number, 1.0 / nekrs::referenceSource(), &_source_elem);
    _nek_problem.writeVolumeSolution(e, field::heat_source, _source_elem);
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

    _console << "[volume]: Normalizing total NekRS heat source of "
           << Moose::stringify(nek_source * nek_source_print_mult)
           << " to the conserved MOOSE value of " + Moose::stringify(moose_source) << std::endl;

  // Any unit changes (for DIMENSIONAL nekRS runs) are automatically accounted for
  // here because moose_source is an integral on the MOOSE mesh, while nek_source is
  // an integral on the nek mesh
  successful_normalization = normalizeVolumetricSource(moose_source, nek_source, normalized_nek_source);

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

#endif
