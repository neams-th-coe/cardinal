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

#ifdef ENABLE_OPENMC_COUPLING

#include "OpenMCCellAverageProblem.h"
#include "AuxiliarySystem.h"
#include "DelimitedFileReader.h"
#include "TimedPrint.h"
#include "MooseUtils.h"
#include "MooseMeshUtils.h"
#include "NonlinearSystemBase.h"
#include "Conversion.h"
#include "VariadicTable.h"
#include "UserErrorChecking.h"

#include "mpi.h"
#include "openmc/capi.h"
#include "openmc/cell.h"
#include "openmc/constants.h"
#include "openmc/cross_sections.h"
#include "openmc/dagmc.h"
#include "openmc/error.h"
#include "openmc/particle.h"
#include "openmc/geometry.h"
#include "openmc/geometry_aux.h"
#include "openmc/material.h"
#include "openmc/message_passing.h"
#include "openmc/nuclide.h"
#include "openmc/random_lcg.h"
#include "openmc/settings.h"
#include "openmc/summary.h"
#include "openmc/tallies/trigger.h"
#include "openmc/volume_calc.h"
#include "openmc/universe.h"
#include "xtensor/xarray.hpp"
#include "xtensor/xview.hpp"

registerMooseObject("CardinalApp", OpenMCCellAverageProblem);

bool OpenMCCellAverageProblem::_first_transfer = true;

InputParameters
OpenMCCellAverageProblem::validParams()
{
  InputParameters params = OpenMCProblemBase::validParams();
  params.addParam<std::vector<SubdomainName>>(
      "fluid_blocks",
      "Subdomain ID(s) corresponding to the fluid phase, "
      "for which both density and temperature will be sent to OpenMC");
  params.addParam<std::vector<SubdomainName>>("solid_blocks",
                                              "Subdomain ID(s) corresponding to the solid phase, "
                                              "for which temperature will be sent to OpenMC");
  params.addParam<std::vector<SubdomainName>>(
      "tally_blocks",
      "Subdomain ID(s) for which to add tallies in the OpenMC model; "
      "only used with cell tallies");
  params.addParam<bool>("check_tally_sum",
                        "Whether to check consistency between the local tallies "
                        "with a global tally");
  params.addParam<bool>(
      "check_zero_tallies",
      true,
      "Whether to throw an error if any tallies from OpenMC evaluate to zero; "
      "this can be helpful in reducing the number of tallies if you inadvertently add tallies "
      "to a non-fissile region, or for catching geomtery setup errors");
  params.addParam<MooseEnum>(
      "initial_properties",
      getInitialPropertiesEnum(),
      "Where to read the temperature and density initial conditions for the OpenMC model");

  params.addParam<bool>(
      "export_properties",
      false,
      "Whether to export OpenMC's temperature and density properties after updating "
      "them from MOOSE.");
  params.addParam<bool>(
      "normalize_by_global_tally",
      true,
      "Whether to normalize by a global tally (true) or else by the sum "
      "of the local tally (false)");
  params.addParam<bool>(
      "assume_separate_tallies",
      false,
      "Whether to assume that all tallies added by in the XML files and automatically "
      "by Cardinal are spatially separate. This is a performance optimization");

  params.addParam<bool>("map_density_by_cell",
      true,
      "Whether to apply a unique density to every OpenMC cell (the default), or "
      "instead apply a unique density to every OpenMC material (even if that material is "
      "filled into more than one cell). If your OpenMC model has a unique material "
      "in every cell you want to receive density feedback, these two options are IDENTICAL");

  // TODO: would be nice to auto-detect this
  params.addParam<bool>("fixed_mesh", true,
    "Whether the MooseMesh is unchanging during the simulation (true), or whether there is mesh "
    "movement and/or adaptivity that is changing the mesh in time (false). When the mesh changes "
    "during the simulation, the mapping from OpenMC's cells to the mesh must be re-evaluated after "
    "each OpenMC run.");

  params.addRequiredParam<MooseEnum>(
      "tally_type", getTallyTypeEnum(), "Type of tally to use in OpenMC");
  params.addParam<MooseEnum>(
      "tally_estimator", getTallyEstimatorEnum(), "Type of tally estimator to use in OpenMC");

  params.addParam<MultiMooseEnum>(
      "tally_score", getTallyScoreEnum(), "Score(s) to use in the OpenMC tallies. If not specified, defaults to 'kappa_fission'");

  MooseEnum scores_heat(
    "heating heating_local kappa_fission fission_q_prompt fission_q_recoverable");
  params.addParam<MooseEnum>("source_rate_normalization", scores_heat, "Score to use for computing the "
      "particle source rate (source/sec) for a certain tallies in eigenvalue mode. In other words, the "
      "source/sec is computed as power / the global value of this tally");

  params.addParam<std::vector<std::string>>(
      "tally_name", "Auxiliary variable name(s) to use for OpenMC tallies. "
      "If not specified, defaults to the names of the scores");
  params.addParam<std::string>("mesh_template",
                               "Mesh tally template for OpenMC when using mesh tallies; "
                               "at present, this mesh must exactly match the mesh used in the "
                               "[Mesh] block because a one-to-one copy "
                               "is used to get OpenMC's tally results on the [Mesh] in preparation "
                               "for transfer to another App.");
  params.addParam<std::vector<Point>>("mesh_translations",
                                      "Coordinates to which each mesh template should be "
                                      "translated, if multiple unstructured meshes "
                                      "are desired.");
  params.addParam<std::vector<FileName>>("mesh_translations_file",
                                         "File providing the coordinates to which each mesh "
                                         "template should be translated, if multiple "
                                         "unstructured meshes are desired.");

  params.addParam<MooseEnum>("tally_trigger",
                             getTallyTriggerEnum(),
                             "Trigger criterion to determine when OpenMC simulation is complete "
                             "based on tallies. If multiple scores are specified in 'tally_score, "
                             "this same trigger is applied to all scores.");
  params.addRangeCheckedParam<Real>(
      "tally_trigger_threshold", "tally_trigger_threshold > 0", "Threshold for the tally trigger");
  params.addParam<MooseEnum>(
      "k_trigger",
      getTallyTriggerEnum(),
      "Trigger criterion to determine when OpenMC simulation is complete based on k");
  params.addRangeCheckedParam<Real>(
      "k_trigger_threshold", "k_trigger_threshold > 0", "Threshold for the k trigger");
  params.addRangeCheckedParam<unsigned int>(
      "max_batches", "max_batches > 0", "Maximum number of batches, when using triggers");
  params.addRangeCheckedParam<unsigned int>(
      "batch_interval", 1, "batch_interval > 0", "Trigger batch interval");

  params.addParam<std::vector<std::vector<std::string>>>(
      "temperature_variables",
      "Vector of variable names corresponding to the temperatures sent into OpenMC. Each entry maps to "
      "the corresponding entry in 'temperature_blocks.' If not specified, each entry defaults to 'temp'");
  params.addParam<std::vector<std::vector<SubdomainName>>>(
      "temperature_blocks", "Blocks corresponding to each of the 'temperature_variables'. If not specified, "
      "defaults to the set union of 'fluid_blocks' and 'solid_blocks'");

  params.addParam<bool>(
      "check_equal_mapped_tally_volumes",
      false,
      "Whether to check if the tallied cells map to regions in the mesh of equal volume. "
      "This can be helpful to ensure that the volume normalization of OpenMC's tallies doesn't "
      "introduce any unintentional distortion just because the mapped volumes are different. "
      "You should only set this to true if your OpenMC tally cells are all the same volume!");
  params.addRangeCheckedParam<Real>("equal_tally_volume_abs_tol", 1e-8, "equal_tally_volume_abs_tol > 0",
      "Absolute tolerance for comparing tally volumes");

  params.addParam<unsigned int>("solid_cell_level",
                                "Coordinate level in OpenMC to use for identifying solid cells");
  params.addParam<unsigned int>(
      "lowest_solid_cell_level",
      "Lowest coordinate level in OpenMC to use for identifying solid cells");

  params.addParam<unsigned int>(
      "fluid_cell_level", "Coordinate level in OpenMC to stop at for identifying fluid cells");
  params.addParam<unsigned int>(
      "lowest_fluid_cell_level",
      "Lowest coordinate level in OpenMC to use for identifying fluid cells");

  params.addParam<bool>(
      "identical_tally_cell_fills",
      false,
      "Whether the tallied cells have identical "
      "fill universes; this is an optimization to speed up initialization for TRISO problems "
      "where each TRISO pebble/compact/plate/etc. has exactly the same universe filling it.");
  params.addParam<bool>(
      "check_identical_tally_cell_fills",
      false,
      "Whether to check that your model does indeed have identical tally cell fills, allowing "
      "you to set 'identical_tally_cell_fills = true' to speed up initialization");

  MultiMooseEnum openmc_outputs("unrelaxed_tally_std_dev unrelaxed_tally");
  params.addParam<MultiMooseEnum>(
      "output", openmc_outputs, "UNRELAXED field(s) to output from OpenMC for each tally score. "
      "unrelaxed_tally_std_dev will write the standard deviation of each tally into auxiliary variables "
      "named *_std_dev. Unrelaxed_tally will write the raw unrelaxed tally into auxiliary variables "
      "named *_raw (replace * with 'tally_name').");
  params.addParam<std::vector<std::string>>("output_name", "Auxiliary variable name(s) to write "
    "'output' into. This class now sets up the variable names automatically so this parameter is unused.");

  params.addParam<MooseEnum>("relaxation",
                             getRelaxationEnum(),
                             "Type of relaxation to apply to the OpenMC solution");
  params.addRangeCheckedParam<Real>("relaxation_factor",
                                    0.5,
                                    "relaxation_factor > 0.0 & relaxation_factor < 2.0",
                                    "Relaxation factor for use with constant relaxation");
  params.addParam<int>("first_iteration_particles",
                       "Number of particles to use for first iteration "
                       "when using Dufek-Gudowski relaxation");

  params.addParam<UserObjectName>("symmetry_mapper", "User object (of type SymmetryPointGenerator) "
    "to map from a symmetric OpenMC model to a full-domain [Mesh]. For example, you can use this "
    "to map from a quarter-symmetric OpenMC model to a whole-domain [Mesh].");
  params.addParam<Point>("symmetry_plane_normal",
               "Normal that defines a symmetry plane in the OpenMC model");
  params.addParam<Point>("symmetry_axis",
               "Axis about which to rotate for angle-symmetric OpenMC models");
  params.addRangeCheckedParam<Real>(
      "symmetry_angle",
      "symmetry_angle > 0 & symmetry_angle <= 180",
      "Angle (degrees) from symmetry plane for which OpenMC model is symmetric");

  params.addParam<UserObjectName>("volume_calculation",
    "An optional user object that will perform a stochastic volume calculation to get the OpenMC "
    "cell volumes. This can be used to check that the MOOSE regions to which the cells map are "
    "of approximately the same volume.");
  params.addParam<UserObjectName>("skinner", "When using DAGMC geometries, an optional skinner that will "
    "regenerate the OpenMC geometry on-the-fly according to iso-contours of temperature and density");
  return params;
}

OpenMCCellAverageProblem::OpenMCCellAverageProblem(const InputParameters & params)
  : OpenMCProblemBase(params),
    _serialized_solution(NumericVector<Number>::build(_communicator).release()),
    _tally_type(getParam<MooseEnum>("tally_type").getEnum<tally::TallyTypeEnum>()),
    _initial_condition(
        getParam<MooseEnum>("initial_properties").getEnum<coupling::OpenMCInitialCondition>()),
    _relaxation(getParam<MooseEnum>("relaxation").getEnum<relaxation::RelaxationEnum>()),
    _tally_trigger(getParam<MooseEnum>("tally_trigger").getEnum<tally::TallyTriggerTypeEnum>()),
    _k_trigger(getParam<MooseEnum>("k_trigger").getEnum<tally::TallyTriggerTypeEnum>()),
    _check_zero_tallies(getParam<bool>("check_zero_tallies")),
    _export_properties(getParam<bool>("export_properties")),
    _normalize_by_global(_run_mode == openmc::RunMode::FIXED_SOURCE ? false :
                                      getParam<bool>("normalize_by_global_tally")),
    _need_to_reinit_coupling(!getParam<bool>("fixed_mesh")),
    _check_tally_sum(isParamValid("check_tally_sum") ? getParam<bool>("check_tally_sum") :
                                                       (_run_mode == openmc::RunMode::FIXED_SOURCE ?
                                                        true : _normalize_by_global)),
    _check_equal_mapped_tally_volumes(getParam<bool>("check_equal_mapped_tally_volumes")),
    _equal_tally_volume_abs_tol(getParam<Real>("equal_tally_volume_abs_tol")),
    _relaxation_factor(getParam<Real>("relaxation_factor")),
    _identical_tally_cell_fills(getParam<bool>("identical_tally_cell_fills")),
    _check_identical_tally_cell_fills(getParam<bool>("check_identical_tally_cell_fills")),
    _assume_separate_tallies(getParam<bool>("assume_separate_tallies")),
    _map_density_by_cell(getParam<bool>("map_density_by_cell")),
    _has_fluid_blocks(params.isParamSetByUser("fluid_blocks")),
    _has_solid_blocks(params.isParamSetByUser("solid_blocks")),
    _needs_global_tally(_check_tally_sum || _normalize_by_global),
    _volume_calc(nullptr),
    _symmetry(nullptr)
{
  // We need to clear and re-initialize the OpenMC tallies if
  // fixed_mesh is false, which indicates at least one of the following:
  //   - the [Mesh] is being adaptively refined
  //   - the [Mesh] is deforming in space
  //
  // If the [Mesh] is changing, then we certainly know that the mesh tallies
  // need to be re-initialized because (a) for file-based mesh tallies, we need
  // to enforce that the mesh is identical to the [Mesh] and (b) for directly
  // tallying on the [Mesh], we need to pass that mesh info into OpenMC. For good
  // measure, we also need to re-initialize cell tallies because it's possible
  // that as the [Mesh] changes, the mapping from OpenMC cells to the [Mesh]
  // also changes, which could open the door to new cell IDs/instances being added
  // to the cell instance filter. If we need to re-init tallies, then we can't
  // guarantee that the tallies from iteration to iteration correspond to exactly
  // the same number of bins or to exactly the same regions of space, so we must
  // disable relaxation.
  if (_need_to_reinit_coupling && _relaxation != relaxation::none)
    mooseError("When 'fixed_mesh' is false, the mapping from the OpenMC model to the [Mesh] may "
      "vary in time. This means that we have no guarantee that the number of tally bins (or even "
      "the regions of space corresponding to each bin) are fixed. Therefore, it is not "
      "possible to apply relaxation to the OpenMC tallies because you might end up trying to add vectors "
      "of different length (and possibly spatial mapping).");

  if (_run_mode == openmc::RunMode::FIXED_SOURCE)
    checkUnusedParam(params, "normalize_by_global_tally", "running OpenMC in fixed source mode");

  if (isParamValid("tally_estimator"))
  {
    auto estimator = getParam<MooseEnum>("tally_estimator").getEnum<tally::TallyEstimatorEnum>();
    if (_tally_type == tally::mesh && estimator == tally::tracklength)
      mooseError("Tracklength estimators are currently incompatible with mesh tallies!");

    _tally_estimator = tallyEstimator(estimator);
  }
  else
  {
    // set a default of tracklength for cells, and use mandatory collision for mesh tallies
    switch (_tally_type)
    {
      case tally::cell:
        _tally_estimator = openmc::TallyEstimator::TRACKLENGTH;
        break;
      case tally::mesh:
        _tally_estimator = openmc::TallyEstimator::COLLISION;
        break;
      default:
        mooseError("Unhandled TallyTypeEnum in OpenMCCellAverageProblem!");
    }
  }

  if (_run_mode != openmc::RunMode::EIGENVALUE && _k_trigger != tally::none)
    mooseError("Cannot specify a 'k_trigger' for OpenMC runs that are not eigenvalue mode!");

  if (isParamValid("tally_score"))
  {
    auto scores = getParam<MultiMooseEnum>("tally_score");
    for (const auto & score : scores)
      _tally_score.push_back(tallyScore(score));
  }
  else
    _tally_score = {"kappa-fission"};

  if (std::find(_tally_score.begin(), _tally_score.end(), "heating") != _tally_score.end())
    if (!openmc::settings::photon_transport)
      mooseWarning("When using the 'heating' score with photon transport disabled, energy deposition\n"
        "from photons is neglected unless you specifically ran NJOY to produce MT=301 with\n"
        "photon energy deposited locally (not true for any pre-packaged OpenMC data libraries\n"
        "on openmc.org).\n\n"
        "If you did NOT specifically run NJOY yourself with this customization, we recommend\n"
        "using the 'heating_local' score instead, which will capture photon energy deposition.\n"
        "Otherwise, you will underpredict the true energy deposition.");

  // need some special treatment for non-heating scores, in eigenvalue mode
  bool has_non_heating_score = false;
  for (const auto & t : _tally_score)
    if (!isHeatingScore(t))
      has_non_heating_score = true;

  if (has_non_heating_score && _run_mode == openmc::RunMode::EIGENVALUE)
  {
    std::string non_heating_scores;
    for (const auto & e : _tally_score)
    {
      if (!isHeatingScore(e))
      {
        std::string l = e;
        std::replace(l.begin(), l.end(), '-', '_');
        non_heating_scores += "" + l + ", ";
      }
    }

    if (non_heating_scores.length() > 0)
      non_heating_scores.erase(non_heating_scores.length() - 2);

    checkRequiredParam(params, "source_rate_normalization", "using a non-heating tally (" +
      non_heating_scores + ") in eigenvalue mode");
    auto norm = getParam<MooseEnum>("source_rate_normalization");

    // If the score is already in tally_score, no need to do anything special.
    // Otherwise, we need to add that score.
    std::string n = norm;
    std::replace(n.begin(), n.end(), '_', '-');
    auto it = std::find(_tally_score.begin(), _tally_score.end(), n);
    if (it != _tally_score.end())
      _source_rate_index = it - _tally_score.begin();
    else
    {
      _tally_score.push_back(tallyScore(n));
      _source_rate_index = _tally_score.size() - 1;

      if (isParamValid("tally_name"))
        mooseError("When specifying 'tally_name', the score indicated in 'source_rate_normalization' must be\n"
          "listed in 'tally_score' so that we know what you want to name that score (", tallyScore(n), ")");
    }
  }
  else
    checkUnusedParam(params, "source_rate_normalization", "either running in fixed-source mode, or all tallies have units of eV/src");

  if (isParamValid("tally_name"))
    _tally_name = getParam<std::vector<std::string>>("tally_name");
  else
  {
    for (auto score : _tally_score)
    {
      std::replace(score.begin(), score.end(), '-', '_');
      _tally_name.push_back(score);
    }
  }

  if (_tally_name.size() != _tally_score.size())
    mooseError("'tally_name' must be the same length as 'tally_score'!");

  if (has_non_heating_score && _run_mode == openmc::RunMode::EIGENVALUE)
  {
    // later, we populate the tally results in a loop. We will rely on the normalization
    // tally being listed before the non-heating tally, so we swap entries so that the normalization
    // tally is first
    std::iter_swap(_tally_score.begin(), _tally_score.begin() + _source_rate_index);
    std::iter_swap(_tally_name.begin(), _tally_name.begin() + _source_rate_index);
  }

  _source_rate_index = 0;

  std::set<std::string> name(_tally_name.begin(), _tally_name.end());
  std::set<std::string> score(_tally_score.begin(), _tally_score.end());
  if (_tally_name.size() != name.size())
    mooseError("'tally_name' cannot contain duplicate entries!");

  if (_tally_score.size() != score.size())
    mooseError("'tally_score' cannot contain duplicate entries!");

  if (_tally_type == tally::mesh)
    if (_mesh.getMesh().allow_renumbering() && !_mesh.getMesh().is_replicated())
      mooseError("Mesh tallies currently require 'allow_renumbering = false' to be set in the [Mesh]!");

  if (isParamValid("symmetry_plane_normal") || isParamValid("symmetry_axis") || isParamValid("symmetry_angle"))
    mooseError("The 'symmetry_plane_normal', 'symmetry_axis', and 'symmetry_angle' functionality has been moved into the SymmetryPointGenerator user object. Please add a SymmetryPointGenerator user object and pass into the 'symmetry_mapper' parameter.");

  if (_assume_separate_tallies && _needs_global_tally)
    paramError("assume_separate_tallies",
               "Cannot assume separate tallies when either of 'check_tally_sum' or"
               "'normalize_by_global_tally' is true!");

  // determine the number of particles set either through XML or the wrapping
  if (_relaxation == relaxation::dufek_gudowski)
  {
    checkUnusedParam(params, "particles", "using Dufek-Gudowski relaxation");
    checkRequiredParam(params, "first_iteration_particles", "using Dufek-Gudowski relaxation");
    openmc::settings::n_particles = getParam<int>("first_iteration_particles");
  }
  else
    checkUnusedParam(params, "first_iteration_particles", "not using Dufek-Gudowski relaxation");

   if (!_has_fluid_blocks || isParamValid("skinner"))
     checkUnusedParam(params, "map_density_by_cell", "either (i) applying geometry skinning or (ii) 'fluid_blocks' is empty");

  // OpenMC will throw an error if the geometry contains DAG universes but OpenMC wasn't compiled with DAGMC.
  // So we can assume that if we have a DAGMC geometry, that we will also by this point have DAGMC enabled.
#ifdef ENABLE_DAGMC
  bool has_csg;
  bool has_dag;
  geometryType(has_csg, has_dag);

  if (!has_dag)
    checkUnusedParam(params, "skinner", "the OpenMC model does not contain any DagMC universes");
  else if (isParamValid("skinner"))
  {
    // TODO: we currently delete the entire OpenMC geometry, and only re-build the cells
    // bounded by the skins. We can generalize this later to only regenerate DAGMC universes,
    // so that CSG cells are untouched by the skinner. We'd also need to be careful with the
    // relationships between the DAGMC universes and the CSG cells, because the DAGMC universes
    // could be filled inside of the CSG cells.
    if (has_csg && has_dag)
      mooseError("The 'skinner' can only be used with OpenMC geometries that are entirely DAGMC based.\n"
        "Your model contains a combination of both CSG and DAG cells.");

    // We know there will be a single DAGMC universe, because we already impose
    // above that there cannot be CSG cells (and the only way to get >1 DAGMC
    // universe is to fill it inside a CSG cell).
    for (const auto& universe: openmc::model::universes)
      if (universe->geom_type() == openmc::GeometryType::DAG)
        _dagmc_universe_index = openmc::model::universe_map[universe->id_];

    const openmc::Universe * u = openmc::model::universes[_dagmc_universe_index].get();
    const openmc::DAGUniverse * dag = dynamic_cast<const openmc::DAGUniverse *>(u);
    if (dag->uses_uwuw()) // TODO: test
      mooseError("The 'skinner' does not currently support the UWUW workflow.");

    // The newly-generated DAGMC cells could be disjoint in space, in which case
    // it is impossible for us to know with 100% certainty a priori how many materials
    // we would need to create.
    _map_density_by_cell = false;
  }
#else
  checkUnusedParam(params, "skinner", "DAGMC geometries in OpenMC are not enabled in this build of Cardinal");
#endif

  _n_particles_1 = nParticles();

  // set the parameters needed for tally triggers
  getTallyTriggerParameters(params);

  if (_relaxation != relaxation::constant)
    checkUnusedParam(params, "relaxation_factor", "not using constant relaxation");

  if (!_identical_tally_cell_fills)
    checkUnusedParam(
        params, "check_identical_tally_cell_fills", "'identical_tally_cell_fills' is false");

  if (!isParamValid("fluid_blocks") && !isParamValid("solid_blocks"))
    mooseError("At least one of 'fluid_blocks' and 'solid_blocks' must be specified to "
               "establish the mapping from MOOSE to OpenMC.");

  readFluidBlocks();
  readSolidBlocks();

  if (!isParamValid("temperature_blocks"))
    checkUnusedParam(params, "temperature_variables", "not setting 'temperature_blocks'");

  if (isParamValid("temperature_blocks"))
  {
    auto temperature_blocks = getParam<std::vector<std::vector<SubdomainName>>>("temperature_blocks");
    checkEmptyVector(temperature_blocks, "'temperature_blocks'");
    for (const auto & t : temperature_blocks)
      checkEmptyVector(t, "Entries in 'temperature_blocks'");

    // as sanity check, shouldn't have any entries in temperature_blocks which are not
    // also in fluid_blocks or solid_blocks. TODO: eventually, we will deprecate solid_blocks
    std::vector<SubdomainName> flattened_tb;
    for (const auto & slice : temperature_blocks)
      for (const auto & i : slice)
        flattened_tb.push_back(i);

    auto t_ids = _mesh.getSubdomainIDs(flattened_tb);
    for (const auto & t : t_ids)
      if (!_fluid_blocks.count(t) && !_solid_blocks.count(t))
        mooseError("Each entry in 'temperature_blocks' should be in either 'fluid_blocks' or "
          "'solid_blocks'. Could not find '", t, "' in either!");

    // because this is the mechanism by which we will impose temperatures, we also need to have
    // all the listed blocks match those given in the 'fluid_blocks' and 'solid_blocks', or else
    // that block would secretly not participate in coupling
    for (const auto & t : _fluid_block_names)
    {
      auto it = std::find(flattened_tb.begin(), flattened_tb.end(), t);
      if (it == flattened_tb.end())
        mooseError("Each entry in 'fluid_blocks' must be included in 'temperature_blocks'!\n"
          "Block '", t, "' is in 'fluid_blocks' but not 'temperature_blocks'.");
    }

    for (const auto & t : _solid_block_names)
    {
      auto it = std::find(flattened_tb.begin(), flattened_tb.end(), t);
      if (it == flattened_tb.end())
        mooseError("Each entry in 'solid_blocks' must be included in 'temperature_blocks'!\n"
          "Block '", t, "' is in 'solid_blocks' but not 'temperature_blocks'.");
    }

    // should not be any duplicate blocks, otherwise it is not clear which temperature variable to use
    std::set<SubdomainName> names;
    for (const auto & b : flattened_tb)
    {
      if (names.count(b))
        mooseError("Subdomains cannot be repeated in 'temperature_blocks'! Subdomain '", b, "' is duplicated.");
      names.insert(b);
    }

    // now, get the names of those temperature variables
    std::vector<std::vector<std::string>> temperature_vars;
    if (isParamValid("temperature_variables"))
    {
      temperature_vars = getParam<std::vector<std::vector<std::string>>>("temperature_variables");

      checkEmptyVector(temperature_vars, "'temperature_variables'");
      for (const auto & t : temperature_vars)
        checkEmptyVector(t, "Entries in 'temperature_variables'");

      if (temperature_vars.size() != temperature_blocks.size())
        mooseError("'temperature_variables' and 'temperature_blocks' must be the same length!");

      // TODO: for now, we restrict each set of blocks to map to a single temperature variable
      for (std::size_t i = 0; i < temperature_vars.size(); ++i)
        if (temperature_vars[i].size() > 1)
          mooseError("Each entry in 'temperature_variables' must be of length 1. "
            "Entry " + std::to_string(i) + " is of length ", temperature_vars[i].size(), ".");
    }
    else
    {
      // set a reasonable default, if not specified
      temperature_vars.resize(temperature_blocks.size(), std::vector<std::string>(1));
      for (std::size_t i = 0; i < temperature_blocks.size(); ++i)
        temperature_vars[i][0] = "temp";
    }

    for (std::size_t i = 0; i < temperature_vars.size(); ++i)
      for (std::size_t j = 0; j < temperature_blocks[i].size(); ++j)
        _temp_vars_to_blocks[temperature_vars[i][0]].push_back(temperature_blocks[i][j]);
  }
  else
  {
    // default to the union of fluid_blocks and solid_blocks, all being named 'temp'
    for (const auto & b : _solid_block_names)
      _temp_vars_to_blocks["temp"].push_back(b);
    for (const auto & b : _fluid_block_names)
      _temp_vars_to_blocks["temp"].push_back(b);
  }

  switch (_tally_type)
  {
    case tally::cell:
    {
      checkUnusedParam(params, {"mesh_template", "mesh_translations", "mesh_translations_file"},
                               "using cell tallies");

      readTallyBlocks();

      // If not specified, add tallies to all MOOSE blocks
      if (!isParamValid("tally_blocks"))
        for (const auto & s : _mesh.meshSubdomains())
          _tally_blocks.insert(s);

      break;
    }
    case tally::mesh:
    {
      checkUnusedParam(params, "tally_blocks", "using mesh tallies");

      if (isParamValid("mesh_template"))
      {
        _mesh_template_filename = &getParam<std::string>("mesh_template");

        if (isParamValid("mesh_translations") && isParamValid("mesh_translations_file"))
          mooseError("Both 'mesh_translations' and 'mesh_translations_file' cannot be specified");
      }
      else
      {
         if (std::abs(_scaling - 1.0) > 1e-6)
           mooseError("Directly tallying on the [Mesh] is only supported for 'scaling' of unity,\n"
             "because we multiply the [Mesh] by 'scaling' when tallying on it in OpenMC.");

         // for distributed meshes, each rank only owns a portion of the mesh information, but
         // OpenMC wants the entire mesh to be available on every rank. We might be able to add
         // this feature in the future, but will need to investigate
         if (!_mesh.getMesh().is_replicated())
           mooseError("Directly tallying on the [Mesh] block by OpenMC is not yet supported "
             "for distributed meshes!");

        // if user does not provide a 'mesh_template', just use the [Mesh] block, which means these
        // other parameters are ignored. To simplify logic elsewhere in the code, we throw an error
        if (isParamValid("mesh_translations"))
          mooseError("When reading the tally mesh from the [Mesh] block, the 'mesh_translations' "
            "cannot be specified!");

        if (isParamValid("mesh_translations_file"))
          mooseError("When reading the tally mesh from the [Mesh] block, the 'mesh_translations_file' "
            "cannot be specified!");
      }

      if (_check_equal_mapped_tally_volumes)
        mooseWarning(
            "The 'check_equal_mapped_tally_volumes' parameter is unused when using mesh tallies!");

      fillMeshTranslations();

      break;
    }
    default:
      mooseError("Unhandled TallyTypeEnum in OpenMCCellAverageProblem!");
  }

  // Make sure the same block ID doesn't appear in both the fluid and solid blocks,
  // or else we won't know how to send feedback into OpenMC.
  checkBlockOverlap();

  // get the coordinate level to find cells on for each phase, and warn if invalid or not used
  _using_lowest_solid_level = isParamValid("lowest_solid_cell_level");
  _using_lowest_fluid_level = isParamValid("lowest_fluid_cell_level");
  getCellLevel("fluid", _fluid_cell_level);
  getCellLevel("solid", _solid_cell_level);

  if (isParamValid("output"))
  {
    _outputs = &getParam<MultiMooseEnum>("output");

    if (isParamValid("output_name"))
      mooseError("The 'output_name' is deprecated. We now name the 'output' variables automatically. "
        "Please check the documentation to see what the new names are, or look at the header "
        "at the start of your Cardinal console output.");

    // names of output are appended to ends of 'tally_name'
    for (const auto & o : *_outputs)
    {
      std::string name = o;

      if (o == "UNRELAXED_TALLY_STD_DEV")
        _output_name.push_back("std_dev");
      else if (o == "UNRELAXED_TALLY")
        _output_name.push_back("raw");
      else
        mooseError("Unhandled OutputEnum in OpenMCCellAverageProblem!");
    }
  }
}

void
OpenMCCellAverageProblem::initialSetup()
{
  OpenMCProblemBase::initialSetup();

  if (isParamValid("volume_calculation"))
  {
    auto name = getParam<UserObjectName>("volume_calculation");
    auto base = &getUserObject<UserObject>(name);

    _volume_calc = dynamic_cast<OpenMCVolumeCalculation *>(base);

    if (!_volume_calc)
      paramError("volume_calculation", "The 'volume_calculation' user object must be of type "
        "OpenMCVolumeCalculation!");
  }

  if (_adaptivity.isOn() && !_need_to_reinit_coupling)
    mooseError("When using mesh adaptivity, 'fixed_mesh' must be false!");

  if (isParamValid("symmetry_mapper"))
  {
    auto name = getParam<UserObjectName>("symmetry_mapper");
    auto base = &getUserObject<UserObject>(name);

    _symmetry = dynamic_cast<SymmetryPointGenerator *>(base);

    if (!_symmetry)
      mooseError("The 'symmetry_mapper' user object has to be of type SymmetryPointGenerator!");
  }

  setupProblem();

#ifdef ENABLE_DAGMC
  if (isParamValid("skinner"))
  {
    if (_has_fluid_blocks && _has_solid_blocks)
      mooseError("The 'skinner' currently does not distinguish between fluid vs. solid blocks "
        "(and will apply density skinning over the entire domain). For now, just set your entire "
        "domain to fluid, by setting a density on the MOOSE side to send into OpenMC.");

    if (_symmetry)
      mooseError("Cannot combine the 'skinner' with 'symmetry_mapper'!\n\nWhen using a skinner, "
        "the [Mesh] must exactly match the underlying OpenMC model, so there is\n"
        "no need to transform spatial coordinates to map between OpenMC and the [Mesh].");

    auto name = getParam<UserObjectName>("skinner");
    auto base = &getUserObject<UserObject>(name);

    _skinner = dynamic_cast<MoabSkinner *>(base);

    if (!_skinner)
      paramError("skinner", "The 'skinner' user object must be of type MoabSkinner!");

    if (_skinner->hasDensitySkinning() != _has_fluid_blocks)
      mooseError("Detected inconsistent settings for density skinning and 'fluid_blocks'. If applying "
        "density feedback with 'fluid_blocks', then you must apply density skinning in the '",
        name, "' user object (and vice versa)");

    if (_initial_condition == coupling::hdf5)
      paramError("initial_properties", "Cannot load initial temperature and density properties from "
        "HDF5 files because there is no guarantee that the geometry (which is adaptively changing) matches "
        "that used to write the HDF5 file.");

    _skinner->setGraveyard(true);
    _skinner->setScaling(_scaling);
    _skinner->setVerbosity(_verbose);
    _skinner->makeDependentOnExternalAction();

    // the skinner expects that there is one OpenMC material per subdomain (otherwise this
    // indicates that our [Mesh] doesn't match the .h5m model, because DAGMC itself imposes
    // the one-material-per-cell case. In the future, if we generate DAGMC models directly
    // from the [Mesh] (bypassing the .h5m), we would not need this error check.
    _skinner->setMaterialNames(getMaterialInEachSubdomain());
    _skinner->initialize();
  }
#endif
}

std::vector<std::string>
OpenMCCellAverageProblem::getMaterialInEachSubdomain() const
{
  std::vector<std::string> mats;
  for (const auto & s : _subdomain_to_material)
  {
    if (s.second.size() > 1)
    {
      std::stringstream msg;
      msg << "The 'skinner' expects to find one OpenMC material mapped to each [Mesh] subdomain, but " <<
        Moose::stringify(s.second.size()) << " materials\nmapped to subdomain " << s.first <<
        ". This indicates your [Mesh] is not " <<
        "consistent with the .h5m model.\n\nThe materials which mapped to subdomain " << s.first << " are:\n";

      for (const auto & m : s.second)
        msg << "\n" << materialName(m);

      mooseError(msg.str());
    }

    mats.push_back(materialName(*(s.second.begin())));
  }

  return mats;
}

void
OpenMCCellAverageProblem::setupProblem()
{
  // establish the local -> global element mapping for convenience
  _local_to_global_elem.clear();
  for (unsigned int e = 0; e < _mesh.nElem(); ++e)
  {
    const auto * elem = _mesh.queryElemPtr(e);
    if (!isLocalElem(elem))
      continue;

    _local_to_global_elem.push_back(e);
  }

  _n_openmc_cells = numCells();

  initializeElementToCellMapping();

  getMaterialFills();

  // we do this last so that we can at least hit any other errors first before
  // spending time on the costly filled cell caching
  cacheContainedCells();

  // save the number of contained cells for printing in every transfer if verbose
  for (const auto & c : _cell_to_elem)
  {
    auto cell_info = c.first;
    int32_t n_contained = 0;

    for (const auto & cc : _cell_to_contained_material_cells[cell_info])
      n_contained += cc.second.size();

    _cell_to_n_contained[cell_info] = n_contained;
  }

  subdomainsToMaterials();

  initializeTallies();

  checkMeshTemplateAndTranslations();
}

void
OpenMCCellAverageProblem::getTallyTriggerParameters(const InputParameters & parameters)
{
  // parameters needed for tally triggers
  if (_tally_trigger != tally::none)
  {
    if (_tally_trigger == tally::std_dev || _tally_trigger == tally::variance)
      mooseError(
          "Standard deviation and variance tally triggers are not yet supported!\n"
          "There is not a mechanism for OpenMC to use a different threshold for the different\n"
          "bins in the tallies, which would be needed in order to set thresholds in the same\n"
          "units as in the Cardinal input file.");

    checkRequiredParam(parameters, "tally_trigger_threshold", "using tally triggers");

    _tally_trigger_threshold = getParam<Real>("tally_trigger_threshold");
  }
  else
    checkUnusedParam(parameters, "tally_trigger_threshold", "not using tally triggers");

  // parameters needed for k triggers
  if (_k_trigger != tally::none)
  {
    checkRequiredParam(parameters, "k_trigger_threshold", "using a k trigger");
    openmc::settings::keff_trigger.threshold = getParam<Real>("k_trigger_threshold");
  }
  else
    checkUnusedParam(parameters, "k_trigger_threshold", "not using a k trigger");

  if (_k_trigger != tally::none || _tally_trigger != tally::none) // at least one trigger
  {
    openmc::settings::trigger_on = true;
    checkRequiredParam(parameters, "max_batches", "using triggers");

    int err = openmc_set_n_batches(getParam<unsigned int>("max_batches"),
                                   true /* set the max batches */,
                                   true /* add the last batch for statepoint writing */);
    catchOpenMCError(err, "set the maximum number of batches");

    openmc::settings::trigger_batch_interval = getParam<unsigned int>("batch_interval");
  }

  if (_k_trigger == tally::none && _tally_trigger == tally::none) // no triggers
  {
    checkUnusedParam(parameters, "max_batches", "not using triggers");
    checkUnusedParam(parameters, "batch_interval", "not using triggers");
  }
}

template <typename T>
void
OpenMCCellAverageProblem::checkEmptyVector(const std::vector<T> & vector,
                                           const std::string & name) const
{
  if (vector.empty())
    mooseError(name + " cannot be empty!");
}

void
OpenMCCellAverageProblem::fillMeshTranslations()
{
  if (isParamValid("mesh_translations"))
  {
    _mesh_translations = getParam<std::vector<Point>>("mesh_translations");
    checkEmptyVector(_mesh_translations, "mesh_translations");
  }
  else if (isParamValid("mesh_translations_file"))
  {
    std::vector<FileName> mesh_translations_file =
        getParam<std::vector<FileName>>("mesh_translations_file");
    checkEmptyVector(mesh_translations_file, "mesh_translations_file");

    for (const auto & f : mesh_translations_file)
    {
      MooseUtils::DelimitedFileReader file(f, &_communicator);
      file.setFormatFlag(MooseUtils::DelimitedFileReader::FormatFlag::ROWS);
      file.read();

      const std::vector<std::vector<double>> & data = file.getData();
      readMeshTranslations(data);
    }
  }
  else
    _mesh_translations = {Point(0.0, 0.0, 0.0)};

  // convert to appropriate units
  for (auto & m : _mesh_translations)
    m *= _scaling;
}

void
OpenMCCellAverageProblem::checkMeshTemplateAndTranslations()
{
  // we can do some rudimentary checking on the mesh template by comparing the centroid
  // coordinates compared to centroids in the [Mesh] (because right now, we just doing a simple
  // copy transfer that necessitates the meshes to have the same elements in the same order). In
  // other words, you might have two meshes that represent the same geometry, but if you created
  // the solid phase _first_ in Cubit for one mesh, but the fluid phase _first_ in Cubit for the
  // other mesh, even though the geometry is the same, the element ordering would be different.
  unsigned int offset = 0;
  for (unsigned int i = 0; i < _mesh_filters.size(); ++i)
  {
    const auto & filter = _mesh_filters[i];

    for (int e = 0; e < filter->n_bins(); ++e)
    {
      auto elem_ptr = _mesh.queryElemPtr(offset + e);

      // if element is not on this part of the distributed mesh, skip it
      if (!elem_ptr)
        continue;

      auto pt = _mesh_template->centroid(e);
      Point centroid_template = {pt[0], pt[1], pt[2]};

      // The translation applied in OpenMC isn't actually registered in the mesh itself;
      // it is always added on to the point, so we need to do the same here
      centroid_template += _mesh_translations[i];

      // because the mesh template and [Mesh] may be in different units, we need
      // to adjust the [Mesh] by the scaling factor before doing a comparison.
      Point centroid_mesh = elem_ptr->vertex_average() * _scaling;

      // if the centroids are the same except for a factor of 'scaling', then we can
      // guess that the mesh_template is probably not in units of centimeters
      if (_specified_scaling)
      {
        // if scaling was applied correctly, then each calculation of 'scaling' here should equal 1.
        // Otherwise, if they're all the same, then 'scaling_x' is probably the factor by which the
        // mesh_template needs to be multiplied, so we can print a helpful error message
        bool incorrect_scaling = true;
        for (unsigned int j = 0; j < DIMENSION; ++j)
        {
          Real scaling = centroid_mesh(j) / centroid_template(j);
          incorrect_scaling = incorrect_scaling && !MooseUtils::absoluteFuzzyEqual(scaling, 1.0);
        }

        if (incorrect_scaling)
          mooseError("The centroids of the 'mesh_template' differ from the "
                     "centroids of the [Mesh] by a factor of " +
                     Moose::stringify(centroid_mesh(0) / centroid_template(0)) +
                     ".\nDid you forget that the 'mesh_template' must be in "
                     "the same units as the [Mesh]?");
      }

      // check if centroids are the same
      bool different_centroids = false;
      for (unsigned int j = 0; j < DIMENSION; ++j)
        different_centroids = different_centroids || !MooseUtils::absoluteFuzzyEqual(
                                                         centroid_mesh(j), centroid_template(j));

      if (different_centroids)
        mooseError(
            "Centroid for element " + Moose::stringify(offset + e) + " in the [Mesh] (cm): " +
            printPoint(centroid_mesh) + "\ndoes not match centroid for element " +
            Moose::stringify(e) + " in 'mesh_template' " + Moose::stringify(i) +
            " (cm): " + printPoint(centroid_template) +
            "!\n\nThe copy transfer requires that the [Mesh] and 'mesh_template' be identical.");
    }

    offset += filter->n_bins();
  }
}

void
OpenMCCellAverageProblem::readMeshTranslations(const std::vector<std::vector<double>> & data)
{
  for (const auto & d : data)
  {
    if (d.size() != DIMENSION)
      paramError("mesh_translations_file",
                 "All entries in 'mesh_translations_file' "
                 "must contain exactly ",
                 DIMENSION,
                 " coordinates.");

    // DIMENSION will always be 3
    _mesh_translations.push_back(Point(d[0], d[1], d[2]));
  }
}

void
OpenMCCellAverageProblem::checkBlockOverlap()
{
  std::vector<SubdomainID> intersection;
  std::set_intersection(_fluid_blocks.begin(),
                        _fluid_blocks.end(),
                        _solid_blocks.begin(),
                        _solid_blocks.end(),
                        std::back_inserter(intersection));

  if (intersection.size() != 0)
    mooseError("Block " + Moose::stringify(intersection[0]) +
               " cannot be present in both the 'fluid_blocks' and 'solid_blocks'!");
}

void
OpenMCCellAverageProblem::getCellLevel(const std::string name, unsigned int & cell_level)
{
  std::string param_name = name + "_cell_level";
  std::string lowest_param_name = "lowest_" + name + "_cell_level";

  if (isParamValid(name + "_blocks"))
  {
    bool using_single_level = isParamValid(param_name);
    bool using_lowest_level = isParamValid(lowest_param_name);

    if (using_single_level == using_lowest_level)
      paramError(param_name,
                 "When specifying " + name + " blocks for coupling, either '" + param_name +
                     "' or '" + lowest_param_name +
                     "' must be specified. You have given either both or none.");

    std::string selected_param;

    if (using_single_level)
    {
      cell_level = getParam<unsigned int>(param_name);
      selected_param = param_name;
    }
    else if (using_lowest_level)
    {
      cell_level = getParam<unsigned int>(lowest_param_name);
      selected_param = lowest_param_name;
    }

    if (cell_level >= openmc::model::n_coord_levels)
      paramError(selected_param,
                 "Coordinate level for finding cells cannot be greater than total number "
                 "of coordinate levels: " +
                     Moose::stringify(openmc::model::n_coord_levels) + "!");
  }
  else
  {
    if (isParamValid(param_name))
      mooseWarning("Without setting any '" + name + "_blocks', the '" + param_name +
                   "' parameter is unused!");

    if (isParamValid(lowest_param_name))
      mooseWarning("Without setting any '" + name + "_blocks', the '" + lowest_param_name +
                   "' parameter is unused!");
  }
}

void
OpenMCCellAverageProblem::readBlockParameters(const std::string name,
                                              std::unordered_set<SubdomainID> & blocks,
                                              std::vector<SubdomainName> & names)
{
  std::string param_name = name + "_blocks";

  if (isParamValid(param_name))
  {
    names = getParam<std::vector<SubdomainName>>(param_name);
    checkEmptyVector(names, "'" + param_name + "'");

    auto b_ids = _mesh.getSubdomainIDs(names);

    std::copy(b_ids.begin(), b_ids.end(), std::inserter(blocks, blocks.end()));

    const auto & subdomains = _mesh.meshSubdomains();
    for (const auto & b : blocks)
      if (subdomains.find(b) == subdomains.end())
        mooseError("Block " + Moose::stringify(b) + " specified in '" + name +
                   "_blocks' not found in mesh!");
  }
}

coupling::CouplingFields
OpenMCCellAverageProblem::elemFeedback(const Elem * elem) const
{
  auto id = elem->subdomain_id();

  if (_fluid_blocks.count(id))
    return coupling::density_and_temperature;
  else if (_solid_blocks.count(id))
    return coupling::temperature;
  else
    return coupling::none;
}

void
OpenMCCellAverageProblem::storeElementPhase()
{
  _n_moose_fluid_elems = 0;
  for (const auto & f : _fluid_blocks)
    _n_moose_fluid_elems += numElemsInSubdomain(f);

  _n_moose_solid_elems = 0;
  for (const auto & s : _solid_blocks)
    _n_moose_solid_elems += numElemsInSubdomain(s);

  _n_moose_none_elems = _mesh.nElem() - _n_moose_fluid_elems - _n_moose_solid_elems;
}

void
OpenMCCellAverageProblem::computeCellMappedVolumes()
{
  std::vector<Real> volumes;

  for (const auto & c : _local_cell_to_elem)
  {
    Real vol = 0.0;
    for (const auto & e : c.second)
    {
      // we are looping over local elements, so no need to check for nullptr
      const auto * elem = _mesh.queryElemPtr(globalElemID(e));
      vol += elem->volume();
    }

    volumes.push_back(vol);
  }

  gatherCellSum(volumes, _cell_to_elem_volume);
}

template <typename T>
void
OpenMCCellAverageProblem::gatherCellSum(std::vector<T> & local, std::map<cellInfo, T> & global)
{
  global.clear();
  _communicator.allgather(local);

  for (unsigned int i = 0; i < _flattened_ids.size(); ++i)
  {
    cellInfo cell_info = {_flattened_ids[i], _flattened_instances[i]};

    if (global.count(cell_info))
      global[cell_info] += local[i];
    else
      global[cell_info] = local[i];
  }
}

template <typename T>
void
OpenMCCellAverageProblem::gatherCellVector(std::vector<T> & local, std::vector<unsigned int> & n_local,
  std::map<cellInfo, std::vector<T>> & global)
{
  global.clear();
  _communicator.allgather(n_local);
  _communicator.allgather(local);

  int e = 0;
  for (unsigned int i = 0; i < _flattened_ids.size(); ++i)
  {
    cellInfo cell_info = {_flattened_ids[i], _flattened_instances[i]};

    for (unsigned int j = e; j < e + n_local[i]; ++j)
      global[cell_info].push_back(local[j]);

    e += n_local[i];
  }
}

coupling::CouplingFields
OpenMCCellAverageProblem::cellCouplingFields(const cellInfo & cell_info)
{
  // _cell_to_elem only holds cells that are coupled by feedback to the [Mesh] (for sake of
  // efficiency in cell-based loops for updating temperatures, densities and
  // extracting the tally). But in some auxiliary kernels, we figure out
  // an element's phase in terms of the cell that it maps to. For these cells that
  // do *map* spatially, but just don't participate in coupling, _cell_to_elem doesn't
  // have any notion of those elements
  if (!_cell_phase.count(cell_info))
    return coupling::none;
  else
    return _cell_phase[cell_info];
}

void
OpenMCCellAverageProblem::getCellMappedPhase()
{
  std::vector<int> cells_n_solid;
  std::vector<int> cells_n_fluid;
  std::vector<int> cells_n_none;

  // whether each cell maps to a single phase
  for (const auto & c : _local_cell_to_elem)
  {
    int n_solid = 0, n_fluid = 0, n_none = 0;

    for (const auto & e : c.second)
    {
      // we are looping over local elements, so no need to check for nullptr
      const Elem * elem = _mesh.queryElemPtr(globalElemID(e));

      switch (elemFeedback(elem))
      {
        case coupling::temperature:
          n_solid++;
          break;
        case coupling::density_and_temperature:
          n_fluid++;
          break;
        case coupling::none:
          n_none++;
          break;
        default:
          mooseError("Unhandled CouplingFieldsEnum in OpenMCCellAverageProblem!");
      }
    }

    cells_n_solid.push_back(n_solid);
    cells_n_fluid.push_back(n_fluid);
    cells_n_none.push_back(n_none);
  }

  gatherCellSum(cells_n_solid, _n_solid);
  gatherCellSum(cells_n_fluid, _n_fluid);
  gatherCellSum(cells_n_none, _n_none);
}

Real
OpenMCCellAverageProblem::cellVolume(const cellInfo & cell_info) const
{
  if (_cell_volume.count(cell_info))
    return _cell_volume.at(cell_info);
  else
    return 0.0;
}

void
OpenMCCellAverageProblem::checkCellMappedPhase()
{
  if (_volume_calc)
  {
    _volume_calc->initializeVolumeCalculation();
    _volume_calc->computeVolumes();
  }

  VariadicTable<std::string, int, int, int, std::string, std::string> vt(
      {"Cell", "Solid", "Fluid", "Other", "Mapped Vol", "Actual Vol"});

  // whether the entire problem has identified any fluid or solid cells
  bool has_fluid_cells = false;
  bool has_solid_cells = false;

  std::vector<Real> cv;
  for (const auto & c : _cell_to_elem)
  {
    auto cell_info = c.first;
    int n_solid = _n_solid[cell_info];
    int n_fluid = _n_fluid[cell_info];
    int n_none = _n_none[cell_info];

    std::ostringstream vol;
    vol << std::setprecision(3) << std::scientific << "";
    if (_volume_calc)
    {
      Real v, std_dev;
      _volume_calc->cellVolume(c.first.first, v, std_dev);
      cv.push_back(v);
      vol << v << " +/- " << std_dev;
    }

    std::ostringstream map;
    map << std::setprecision(3) << std::scientific << _cell_to_elem_volume[cell_info];

    // okay to print vol.str() here because only rank 0 is printing (which is the only one
    // with meaningful volume data from OpenMC)
    vt.addRow(printCell(cell_info, true), n_solid, n_fluid, n_none, map.str(), vol.str());

    std::vector<bool> conditions = {n_fluid > 0, n_solid > 0, n_none > 0};
    if (std::count(conditions.begin(), conditions.end(), true) > 1)
    {
      std::stringstream msg;
      msg << "Cell " << printCell(cell_info) << " mapped to " << n_solid << " solid elements, "
          << n_fluid << " fluid elements, and " << n_none
          << " uncoupled elements.\n"
             "Each OpenMC cell, instance pair must map to elements of the same phase.";
      mooseError(msg.str());
    }

    if (n_solid)
    {
      has_solid_cells = true;
      _cell_phase[cell_info] = coupling::temperature;
    }
    else if (n_fluid)
    {
      has_fluid_cells = true;
      _cell_phase[cell_info] = coupling::density_and_temperature;
    }
    else
      _cell_phase[cell_info] = coupling::none;
  }

  // collect values from rank 0 onto all other ranks, then populate cell_volume
  // (this is necessary because in OpenMC, the stochastic volume calculation only
  // gets meaningful results on rank 0
  if (_volume_calc)
  {
    _cell_volume.clear();
    MPI_Bcast(cv.data(), cv.size(), MPI_DOUBLE, 0, _communicator.get());
    int i = 0;
    for (const auto & c : _cell_to_elem)
      _cell_volume[c.first] = cv[i++];
  }

  if (_verbose)
  {
    _console << "\n ===================>     MAPPING FROM OPENMC TO MOOSE     <===================\n" << std::endl;
    _console <<   "          Solid:  # elems in 'solid_blocks' each cell maps to" << std::endl;
    _console <<   "          Fluid:  # elems in 'fluid_blocks' each cell maps to" << std::endl;
    _console <<   "          Other:  # uncoupled elems each cell maps to" << std::endl;
    _console <<   "     Mapped Vol:  volume of MOOSE elems each cell maps to" << std::endl;
    _console <<   "     Actual Vol:  OpenMC cell volume (computed with 'volume_calculation')\n" << std::endl;
    vt.print(_console);

    _console << "\n ===================>     AUXVARIABLES INPUT TO OPENMC     <===================\n" << std::endl;
    _console <<   "      Subdomain:  subdomain name; if unnamed, we show the ID" << std::endl;
    _console <<   "    Temperature:  AuxVariable to read temperature from" << std::endl;
    _console <<   "        Density:  AuxVariable to read density from (empty if no density feedback)\n" << std::endl;

    VariadicTable<std::string, std::string, std::string> aux({"Subdomain", "Temperature", "Density"});

    for (const auto & s : _fluid_block_names)
    {
      auto id = _mesh.getSubdomainID(s);
      aux.addRow(s, _subdomain_to_temp_vars[id].second, _subdomain_to_density_vars[id].second);
    }

    for (const auto & s : _solid_block_names)
    {
      auto id = _mesh.getSubdomainID(s);
      aux.addRow(s, _subdomain_to_temp_vars[id].second, "");
    }

    aux.print(_console);

    _console << "\n ===================>     AUXVARIABLES OUTPUT BY OPENMC     <===================\n" << std::endl;
    _console <<   "    Tally Score:  OpenMC tally score" << std::endl;
    _console <<   "    AuxVariable:  AuxVariable holding this score\n" << std::endl;

    VariadicTable<std::string, std::string> tallies({"Tally Score", "AuxVariable"});
    for (unsigned int i = 0; i < _tally_name.size(); ++i)
      tallies.addRow(_tally_score[i], _tally_name[i]);

    tallies.print(_console);
  }

  if (_has_fluid_blocks && !has_fluid_cells)
    mooseError("'fluid_blocks' was specified, but no fluid elements mapped to OpenMC cells!");

  if (_has_solid_blocks && !has_solid_cells)
    mooseError("'solid_blocks' was specified, but no solid elements mapped to OpenMC cells!");
}

void
OpenMCCellAverageProblem::getCellMappedSubdomains()
{
  std::vector<unsigned int> n_elems;
  std::vector<unsigned int> elem_ids;

  for (const auto & c : _local_cell_to_elem)
  {
    n_elems.push_back(c.second.size());
    for (const auto & e : c.second)
    {
      // we are looping over local elements, so no need to check for nullptr
      const auto * elem = _mesh.queryElemPtr(globalElemID(e));
      elem_ids.push_back(elem->subdomain_id());
    }
  }

  std::map<cellInfo, std::vector<unsigned int>> cell_to_subdomain_vec;
  gatherCellVector(elem_ids, n_elems, cell_to_subdomain_vec);

  // convert to a set
  _cell_to_elem_subdomain.clear();
  for (const auto & c : cell_to_subdomain_vec)
  {
    auto cell_info = c.first;
    for (const auto & s : c.second)
      _cell_to_elem_subdomain[cell_info].insert(s);
  }
}

std::set<SubdomainID>
OpenMCCellAverageProblem::coupledSubdomains()
{
  std::set<SubdomainID> subdomains;
  for (const auto & c : _cell_to_elem)
  {
    auto subdomains_spanning_cell = _cell_to_elem_subdomain[c.first];
    for (const auto & s : subdomains_spanning_cell)
      subdomains.insert(s);
  }

  return subdomains;
}

void
OpenMCCellAverageProblem::subdomainsToMaterials()
{
  for (const auto & c : _cell_to_elem)
  {
    auto contained_cells = _cell_to_contained_material_cells[c.first];
    for (const auto & contained : contained_cells)
    {
      for (const auto & instance : contained.second)
      {
        // we know this is a material cell, so we don't need to check that the fill is material
        int32_t material_index;
        cellInfo cell_info = {contained.first, instance};
        materialFill(cell_info, material_index);

        for (const auto & s : _cell_to_elem_subdomain[c.first])
          _subdomain_to_material[s].insert(material_index);
      }
    }
  }

  VariadicTable<std::string, std::string> vt({"Subdomain", "Material"});
  auto subdomains = coupledSubdomains();
  for (const auto & i : subdomains)
  {
    std::map<std::string, int> mat_to_num;

    for (const auto & m : _subdomain_to_material[i])
    {
      auto name = materialName(m);
      if (mat_to_num.count(name))
        mat_to_num[name] += 1;
      else
        mat_to_num[name] = 1;
    }

    std::string mats = "";
    for (const auto & m : mat_to_num)
    {
      std::string extra = m.second > 1 ? " (" + std::to_string(m.second) + ")" : "";
      mats += " " + m.first + extra + ",";
    }

    mats.pop_back();
    std::string name = _mesh.getSubdomainName(i);
    if (name.empty())
      name = std::to_string(i);
    vt.addRow(name, mats);
  }

  if (_verbose)
  {
    _console << "\n ===================>  OPENMC SUBDOMAIN MATERIAL MAPPING  <====================\n" << std::endl;
    _console <<   "      Subdomain:  Subdomain name; if unnamed, we show the ID" << std::endl;
    _console <<   "       Material:  OpenMC material name(s) in this subdomain; if unnamed, we show the ID." << std::endl;
    _console <<   "                  If N duplicate material names, we show the number in ( ).\n" << std::endl;
    vt.print(_console);
    _console << std::endl;
  }
}

void
OpenMCCellAverageProblem::checkCellMappedSubdomains()
{
  // If the OpenMC cell maps to multiple subdomains that _also_ have different
  // tally settings, we need to error because we are unsure of whether to add tallies or not;
  // both of these need to be true to error
  for (const auto & c : _cell_to_elem)
  {
    bool at_least_one_in_tallies = false;
    bool at_least_one_not_in_tallies = false;
    int block_in_tallies, block_not_in_tallies;

    auto cell_info = c.first;
    for (const auto & s : _cell_to_elem_subdomain[cell_info])
    {
      if (!at_least_one_in_tallies)
      {
        at_least_one_in_tallies = _tally_blocks.count(s) != 0;
        block_in_tallies = s;
      }

      if (!at_least_one_not_in_tallies)
      {
        at_least_one_not_in_tallies = _tally_blocks.count(s) == 0;
        block_not_in_tallies = s;
      }

      // can cut the search early if we've already hit multiple tally settings
      if (at_least_one_in_tallies && at_least_one_not_in_tallies)
        break;
    }

    if (at_least_one_in_tallies && at_least_one_not_in_tallies)
      mooseError("cell " + printCell(cell_info) +
                 " maps to blocks with different tally settings!\n"
                 "Block " +
                 Moose::stringify(block_in_tallies) +
                 " is in 'tally_blocks', but "
                 "block " +
                 Moose::stringify(block_not_in_tallies) + " is not.");

    _cell_has_tally[cell_info] = at_least_one_in_tallies;
  }
}

void
OpenMCCellAverageProblem::getMaterialFills()
{
  VariadicTable<std::string, int> vt({"Cell", "Material"});

  std::set<int32_t> materials_in_fluid;
  std::set<int32_t> other_materials;

  for (const auto & c : _cell_to_elem)
  {
    auto cell_info = c.first;

    int32_t material_index;
    auto is_material_cell = materialFill(cell_info, material_index);

    if (cellCouplingFields(cell_info) != coupling::density_and_temperature)
    {
      // TODO: this check should be extended for non-fluid cells which may contain
      // lattices or  universes
      if (is_material_cell)
        other_materials.insert(material_index);
      continue;
    }

    // check for each material that we haven't already discovered it; if we have, this means we
    // didnt set up the materials correctly (if mapping by cell)
    if (materials_in_fluid.find(material_index) == materials_in_fluid.end())
      materials_in_fluid.insert(material_index);
    else if (_map_density_by_cell)
      mooseError(printMaterial(material_index) + " is present in more than one "
                                                 "fluid cell.\nThis means that your model cannot "
                                                 "independently change the density in cells filled "
                                                 "with this material.\nYou need to edit your OpenMC "
                                                 "model to create additional materials unique to each fluid cell.\n\n"
                                                 "Or, if you want to apply feedback to a material spanning multiple "
                                                 "cells, set 'map_density_by_cell' to false.");


    if (!is_material_cell)
      mooseError("Density transfer does not currently support cells filled with universes or lattices!");

    _cell_to_material[cell_info] = material_index;
    vt.addRow(printCell(cell_info), materialID(material_index));
  }

  if (_verbose && _has_fluid_blocks)
  {
    _console << "\n ===================>       OPENMC MATERIAL MAPPING       <====================\n" << std::endl;
    _console <<   "           Cell:  OpenMC cell receiving density feedback" << std::endl;
    _console <<   "       Material:  OpenMC material ID in this cell\n" << std::endl;
    vt.print(_console);
  }

  // check that the same material is not present in both the density feedback regions and the
  // no-density-feedback regions, because this would give unintended consequences where
  // density is indeed actually changing in parts of the OpenMC model where the user doesn't
  // want that to happen; TODO: we technically should also check that the materials receiving
  // density feedback are not present in parts of the OpenMC which totally do not overlap with
  // the [Mesh] (but we are not tracking their behavior anywhere, we could do this but we'd
  // need to loop over ALL OpenMC cells, get their fills, and check)
  for (const auto & f : materials_in_fluid)
    if (other_materials.count(f))
      mooseError(printMaterial(f) + " is present in more than one OpenMC cell with different "
        "density feedback settings!\nIn other words, this material will have its density changed "
        "by Cardinal (because it is\ncontained in cells which map to the 'fluid_blocks'), but "
        "this material is also present in\nOTHER OpenMC cells, which will give unintended behavior "
        "by changing density in ALL parts of the\ndomain containing this material (some of which have not been coupled via Cardinal).\n\n"
        "Please change your OpenMC model so that unique materials are used in regions which receive "
        "density feedback.");
}

void
OpenMCCellAverageProblem::initializeElementToCellMapping()
{
  /* We consider five different cases here based on how the MOOSE and OpenMC
   * domains might overlap in space:
   *
   * 1: Perfect overlap, every MOOSE element maps to an OpenMC cell and every
   *    OpenMC cell maps to MOOSE element(s)
   *
   * 2: MOOSE domain fully encloses the OpenMC domain, so that not every MOOSE
   *    element maps to an OpenMC cell, but every OpenMC cell maps to a MOOSE element
   *
   * 3: OpenMC domain fully encloses the MOOSE domain, so that not every OpenMC
   *    cell maps to MOOSE element(s), but every MOOSE element maps to an OpenMC cell
   *
   * 4: MOOSE and OpenMC domains only partially overlap, so that not every MOOSE
   *    element maps to an OpenMC and not every OpenMC cell maps to MOOSE element(s)
   *
   * 5: The MOOSE and OpenMC domains do not overlap at all, so no MOOSE elements
   *    map to OpenMC cells and no OpenMC cells map to MOOSE elements.
   *
   * We consider situation #5 to be an error, while the others are technically allowed.
   * We need to error here before getting to OpenMC where we don't map to any cells but
   * would still try to set a cell filter based on no cells.
   */

  // First, figure out the phase of each element according to the blocks defined by the user
  storeElementPhase();

  // perform element to cell mapping
  mapElemsToCells();

  if (!_material_cells_only)
  {
    // gather all cell indices from the initial mapping
    std::vector<int32_t> mapped_cells;
    for (const auto & item : _elem_to_cell)
      mapped_cells.push_back(item.first);

    std::unique(mapped_cells.begin(), mapped_cells.end());
    openmc::prepare_distribcell(&mapped_cells);

    // perform element to cell mapping again to get correct instances
    mapElemsToCells();
  }

  // For each cell, get one point inside it to speed up the particle search
  getPointInCell();

  // Compute the volume that each OpenMC cell maps to in the MOOSE mesh
  computeCellMappedVolumes();

  // Get the number of elements of each phase within the cells
  getCellMappedPhase();

  // Get the element subdomains within each cell
  getCellMappedSubdomains();

  if (_cell_to_elem.size() == 0)
    mooseError("Did not find any overlap between MOOSE elements and OpenMC cells for "
               "the specified blocks!");

  _console << "\nMapping between " + Moose::stringify(_mesh.nElem()) + " MOOSE elements and " +
                  Moose::stringify(_n_openmc_cells) + " OpenMC cells (on " +
                  Moose::stringify(openmc::model::n_coord_levels) + " coordinate levels):"
           << std::endl;

  VariadicTable<std::string, int, int, int, Real> vt(
      {"", "# Solid Elems", "# Fluid Elems", "# Uncoupled Elems", "Length x"});
  vt.addRow("MOOSE mesh", _n_moose_solid_elems, _n_moose_fluid_elems, _n_moose_none_elems, 1.0);
  vt.addRow(
      "OpenMC cells", _n_mapped_solid_elems, _n_mapped_fluid_elems, _n_mapped_none_elems, _scaling);
  vt.print(_console);
  _console << std::endl;

  if (_n_moose_solid_elems && (_n_mapped_solid_elems != _n_moose_solid_elems))
    mooseWarning("The MOOSE mesh has " + Moose::stringify(_n_moose_solid_elems) +
                 " solid elements, "
                 "but only " +
                 Moose::stringify(_n_mapped_solid_elems) + " got mapped to OpenMC cells.");

  if (_n_moose_fluid_elems && (_n_mapped_fluid_elems != _n_moose_fluid_elems))
    mooseWarning("The MOOSE mesh has " + Moose::stringify(_n_moose_fluid_elems) +
                 " fluid elements, "
                 "but only " +
                 Moose::stringify(_n_mapped_fluid_elems) + " got mapped to OpenMC cells.");

  if (_n_mapped_none_elems)
    mooseWarning("Skipping multiphysics feedback for " + Moose::stringify(_n_mapped_none_elems) +
                 " MOOSE elements, " + "which occupy a volume of (cm3): " +
                 Moose::stringify(_uncoupled_volume * _scaling * _scaling * _scaling));

  if (_n_openmc_cells < _cell_to_elem.size())
    mooseError("Internal error: _cell_to_elem has length ", _cell_to_elem.size(), " which should\n"
      "not exceed the number of OpenMC cells, ", _n_openmc_cells);

  // If there is a single coordinate level, we can print a helpful message if there are uncoupled
  // cells in the domain
  auto n_uncoupled_cells = _n_openmc_cells - _cell_to_elem.size();
  if (_single_coord_level && n_uncoupled_cells)
  {
    // Get the number of uncoupled material cells (which we presumably want to include in our
    // coupling). We don't care about VOID cells, because they will never have feedback.
    VariadicTable<std::string, std::string> vt({"Cell", "Contained OpenMC Materials"});

    std::vector<cellInfo> missing_cells;
    int n_missing = 0;
    for (const auto & c : openmc::model::cells)
    {
      // for a single-level geometry, we know that all cells will have just 1 instance,
      // because distributed cells are inherently tied to being repeated (which is not
      // possible with a single universe)
      cellInfo cell {openmc::model::cell_map[c->id_], 0 /* instance */};
      if (!cellIsVoid(cell) && !_cell_to_elem.count(cell))
      {
        n_missing++;

        int32_t material_index;
        materialFill(cell, material_index);
        vt.addRow(printCell(cell), materialName(material_index));
      }
    }

    if (n_missing)
    {
      std::stringstream msg;
      msg << "Skipping multiphysics feedback for " << n_missing <<
             " OpenMC cells!\n\nThis means that there are " << n_missing <<
             " non-void cells in your OpenMC model that will not receive feedback\n"
             "from MOOSE. This is normal if you are intentionally excluding some cells from feedback. The\n"
             "unmapped cells are:\n\n";

      vt.print(msg);
      mooseWarning(msg.str());
    }
  }

  // Check that each cell maps to a single phase
  checkCellMappedPhase();

  // Check that each cell maps to subdomain IDs that all have the same tally setting,
  // and then store which cells should have tallies added to them
  if (_tally_type == tally::cell)
  {
    checkCellMappedSubdomains();
    storeTallyCells();
  }
}

void
OpenMCCellAverageProblem::setContainedCells(const cellInfo & cell_info,
                                            const Point & hint,
                                            std::map<cellInfo, containedCells> & map)
{
  containedCells contained_cells;

  openmc::Position p{hint(0), hint(1), hint(2)};

  const auto & cell = openmc::model::cells[cell_info.first];
  if (cell->type_ == openmc::Fill::MATERIAL)
  {
    std::vector<int32_t> instances = {cell_info.second};
    contained_cells[cell_info.first] = instances;
  }
  else
    contained_cells = cell->get_contained_cells(cell_info.second, &p);

  map[cell_info] = contained_cells;
}

void
OpenMCCellAverageProblem::cacheContainedCells()
{
  TIME_SECTION("cacheContainedCells", 3, "Caching Contained Cells", true);

  // if we're not taking the shortcut assuming each tally cell has identical fills,
  // just compute and then exit
  if (!_identical_tally_cell_fills)
  {
    for (const auto & c : _cell_to_elem)
      setContainedCells(c.first, transformPointToOpenMC(_cell_to_point[c.first]), _cell_to_contained_material_cells);
    return;
  }

  bool first_tally_cell = true;
  bool second_tally_cell = false;
  containedCells first_tally_cell_cc;
  containedCells second_tally_cell_cc;
  containedCells instance_offsets;

  int n = 0;
  for (const auto & c : _cell_to_elem)
  {
    auto cell_info = c.first;
    Point hint = transformPointToOpenMC(_cell_to_point[cell_info]);

    // if the cell doesn't have a tally, default to normal behavior
    if (!_cell_has_tally[cell_info])
      setContainedCells(cell_info, hint, _cell_to_contained_material_cells);
    else
    {
      const auto & cell = openmc::model::cells[cell_info.first];
      if (cell->type_ == openmc::Fill::MATERIAL)
      {
        // behavior is the same for material-filled cells
        setContainedCells(cell_info, hint, _cell_to_contained_material_cells);
      }
      else
      {
        if (first_tally_cell)
        {
          setContainedCells(cell_info, hint, _cell_to_contained_material_cells);
          first_tally_cell_cc = _cell_to_contained_material_cells[cell_info];
          first_tally_cell = false;
          second_tally_cell = true;
        }
        else if (second_tally_cell)
        {
          n++;
          setContainedCells(cell_info, hint, _cell_to_contained_material_cells);
          second_tally_cell_cc = _cell_to_contained_material_cells[cell_info];
          second_tally_cell = false;

          // we will check for equivalence in the end mapping later; but here we still need
          // some checks to make sure the structure is compatible
          checkContainedCellsStructure(cell_info, first_tally_cell_cc, second_tally_cell_cc);

          // get the offset for each instance for each contained cell
          for (const auto & f : first_tally_cell_cc)
          {
            const auto id = f.first;
            const auto & instances = f.second;
            const auto & new_instances = second_tally_cell_cc[id];

            std::vector<int32_t> offsets;
            for (unsigned int i = 0; i < instances.size(); ++i)
              offsets.push_back(new_instances[i] - instances[i]);

            instance_offsets[id] = offsets;
          }
        }
        else
        {
          n++;

          int int_offset = n;

          containedCells contained_cells;
          for (const auto & cc : first_tally_cell_cc)
          {
            const auto & index = cc.first;
            const auto & instances = cc.second;
            auto n_instances = instances.size();
            const auto & shifts = instance_offsets[index];

            std::vector<int32_t> shifted_instances;
            for (unsigned int inst = 0; inst < n_instances; ++inst)
              shifted_instances.push_back(instances[inst] + int_offset * shifts[inst]);

            contained_cells[index] = shifted_instances;
          }

          _cell_to_contained_material_cells[cell_info] = contained_cells;
        }
      }
    }
  }

  // only need to check if we were attempting the shortcut
  if (_check_identical_tally_cell_fills)
  {
    TIME_SECTION("verifyCacheContainedCells", 4, "Verifying Cached Contained Cells", true);

    std::map<cellInfo, containedCells> checking_cell_fills;
    for (const auto & c : _cell_to_elem)
      setContainedCells(c.first, transformPointToOpenMC(_cell_to_point[c.first]), checking_cell_fills);

    std::map<cellInfo, containedCells> ordered_reference(checking_cell_fills.begin(),
                                                         checking_cell_fills.end());
    std::map<cellInfo, containedCells> ordered(_cell_to_contained_material_cells.begin(),
                                               _cell_to_contained_material_cells.end());
    compareContainedCells(ordered_reference, ordered);
  }
}

void
OpenMCCellAverageProblem::checkContainedCellsStructure(const cellInfo & cell_info,
                                                       containedCells & reference,
                                                       containedCells & compare)
{
  // make sure the number of keys is the same
  if (reference.size() != compare.size())
    mooseError("The cell caching failed to identify identical number of cell IDs filling cell " +
               printCell(cell_info) + "\nYou must set 'identical_tally_cell_fills' to false");

  for (const auto & entry : reference)
  {
    auto key = entry.first;

    // check that each key exists
    if (!compare.count(key))
      mooseError("Not all tally cells contain cell ID " + Moose::stringify(cellID(key)) +
                 ". The offender is: cell " + printCell(cell_info) +
                 ".\nYou must set 'identical_tally_cell_fills' to false!");

    // for each int32_t key, compare the std::vector<int32_t> map
    auto reference_instances = entry.second;
    auto compare_instances = compare[key];

    // they should have the same number of instances
    if (reference_instances.size() != compare_instances.size())
      mooseError("The cell caching should have identified " +
                 Moose::stringify(reference_instances.size()) + "cell instances in cell ID " +
                 Moose::stringify(cellID(key)) + ", but instead found " +
                 Moose::stringify(compare_instances.size()) +
                 "\nYou must set 'identical_tally_cell_fills' to false");
    ;
  }
}

void
OpenMCCellAverageProblem::compareContainedCells(std::map<cellInfo, containedCells> & reference,
                                                std::map<cellInfo, containedCells> & compare)
{
  // check that the number of keys matches
  if (reference.size() != compare.size())
    mooseError("The cell caching should have identified " + Moose::stringify(reference.size()) +
               " cells, but instead "
               "found " +
               Moose::stringify(compare.size()));

  // loop over each cellInfo
  for (const auto & entry : reference)
  {
    auto cell_info = entry.first;

    // make sure the key exists
    if (!compare.count(cell_info))
      mooseError("The cell caching failed to map cell " + printCell(cell_info));

    // for each cellInfo key, compare the contained cells map
    auto reference_map = reference[cell_info];
    auto compare_map = compare[cell_info];

    checkContainedCellsStructure(cell_info, reference_map, compare_map);

    // loop over each contained cell
    for (const auto & nested_entry : reference_map)
    {
      // for each int32_t key, compare the std::vector<int32_t> map
      auto reference_instances = nested_entry.second;
      auto compare_instances = compare_map[nested_entry.first];

      std::sort(reference_instances.begin(), reference_instances.end());
      std::sort(compare_instances.begin(), compare_instances.end());

      // and the instances should exactly match
      if (reference_instances != compare_instances)
        mooseError("The cell caching failed to get correct instances for material cell ID " +
                   Moose::stringify(cellID(nested_entry.first)) + " within cell " +
                   printCell(cell_info) + ".\nYou must set 'identical_tally_cell_fills' to false!" +
                   "\n\nThis error might appear if there are OpenMC cells filled with the same "
                   "universe/lattice "
                   "\nfilling the tally cells, but that don't have tallies added to them.");
    }
  }
}

void
OpenMCCellAverageProblem::mapElemsToCells()
{
  // reset counters, flags
  _n_mapped_solid_elems = 0;
  _n_mapped_fluid_elems = 0;
  _n_mapped_none_elems = 0;
  _uncoupled_volume = 0.0;
  _material_cells_only = true;

  // reset data structures
  _elem_to_cell.clear();
  _cell_to_elem.clear();
  _flattened_ids.clear();
  _flattened_instances.clear();

  int local_elem = -1;
  for (unsigned int e = 0; e < _mesh.nElem(); ++e)
  {
    const auto * elem = _mesh.queryElemPtr(e);

    if (!isLocalElem(elem))
      continue;

    local_elem++;

    const Point & c = elem->vertex_average();
    Real element_volume = elem->volume();

    bool error = findCell(c);

    // if we didn't find an OpenMC cell here, then we certainly have an uncoupled region
    if (error)
    {
      _uncoupled_volume += element_volume;
      _n_mapped_none_elems++;
      continue;
    }

    // otherwise, this region may potentially map to OpenMC if we _also_ turned
    // on coupling for this region; first, determine the phase of this element
    // and store the information
    int level;
    auto phase = elemFeedback(elem);

    switch (phase)
    {
      case coupling::density_and_temperature:
      {
        level = _fluid_cell_level;
        _n_mapped_fluid_elems++;

        if (level > _particle.n_coord() - 1)
        {
          if (_using_lowest_fluid_level)
            level = _particle.n_coord() - 1;
          else
            mooseError("Requested coordinate level of " + Moose::stringify(level) +
                       " for the fluid"
                       " exceeds number of nested coordinate levels at " +
                       printPoint(c) + ": " + Moose::stringify(_particle.n_coord()));
        }
        break;
      }
      case coupling::temperature:
      {
        level = _solid_cell_level;
        _n_mapped_solid_elems++;

        if (level > _particle.n_coord() - 1)
        {
          if (_using_lowest_solid_level)
            level = _particle.n_coord() - 1;
          else
            mooseError("Requested coordinate level of " + Moose::stringify(level) +
                       " for the solid"
                       " exceeds number of nested coordinate levels at " +
                       printPoint(c) + ": " + Moose::stringify(_particle.n_coord()));
        }
        break;
      }
      case coupling::none:
      {
        _uncoupled_volume += element_volume;
        _n_mapped_none_elems++;

        // we will succeed in finding a valid cell here; for uncoupled regions,
        // cell_index and cell_instance are unused, so this is just to proceed with program logic
        level = 0;
        break;
      }
      default:
        mooseError("Unhandled CouplingFields enum!");
    }

    auto cell_index = _particle.coord(level).cell;
    auto cell_instance = cell_instance_at_level(_particle, level);

    cellInfo cell_info = {cell_index, cell_instance};

    if (openmc::model::cells[cell_index]->type_ != openmc::Fill::MATERIAL)
      _material_cells_only = false;

    // store the map of cells to elements that will be coupled
    if (phase != coupling::none)
      _cell_to_elem[cell_info].push_back(local_elem);
  }

  _communicator.sum(_n_mapped_solid_elems);
  _communicator.sum(_n_mapped_fluid_elems);
  _communicator.sum(_n_mapped_none_elems);
  _communicator.sum(_uncoupled_volume);

  // if ANY rank finds a non-material cell, they will hold 0 (false)
  _communicator.min(_material_cells_only);

  // store the local mapping of cells to elements for convenience
  _local_cell_to_elem = _cell_to_elem;

  // flatten the cell IDs and instances
  for (const auto & c : _cell_to_elem)
  {
    auto cell_info = c.first;
    _flattened_ids.push_back(cell_info.first);
    _flattened_instances.push_back(cell_info.second);
  }

  _communicator.allgather(_flattened_ids);
  _communicator.allgather(_flattened_instances);

  // collect the _cell_to_elem onto all ranks
  std::vector<unsigned int> n_elems;
  std::vector<unsigned int> elems;
  for (const auto & c : _cell_to_elem)
  {
    n_elems.push_back(c.second.size());
    for (const auto & e : c.second)
      elems.push_back(_local_to_global_elem[e]);
  }

  gatherCellVector(elems, n_elems, _cell_to_elem);

  // fill out the elem_to_cell structure
  _elem_to_cell.resize(_mesh.nElem());
  for (unsigned int e = 0; e < _mesh.nElem(); ++e)
    _elem_to_cell[e] = {UNMAPPED, UNMAPPED};

  for (const auto & c : _cell_to_elem)
  {
    for (const auto & e : c.second)
      _elem_to_cell[e] = c.first;
  }
}

void
OpenMCCellAverageProblem::getPointInCell()
{
  std::vector<Real> x;
  std::vector<Real> y;
  std::vector<Real> z;
  for (const auto & c : _local_cell_to_elem)
  {
    // we are only dealing with local elements here, no need to check for nullptr
    const Elem * elem = _mesh.queryElemPtr(globalElemID(c.second[0]));
    const Point & p = elem->vertex_average();

    x.push_back(p(0));
    y.push_back(p(1));
    z.push_back(p(2));
  }

  _communicator.allgather(x);
  _communicator.allgather(y);
  _communicator.allgather(z);

  // this will get a point from the lowest rank in each cell
  _cell_to_point.clear();
  for (unsigned int i = 0; i < _flattened_ids.size(); ++i)
  {
    cellInfo cell_info = {_flattened_ids[i], _flattened_instances[i]};
    if (!_cell_to_point.count(cell_info))
      _cell_to_point[cell_info] = Point(x[i], y[i], z[i]);
  }
}

void
OpenMCCellAverageProblem::storeTallyCells()
{
  std::stringstream warning;

  bool is_first_tally_cell = true;
  cellInfo first_tally_cell;
  Real mapped_tally_volume;

  _tally_cells.clear();

  for (const auto & c : _cell_to_elem)
  {
    auto cell_info = c.first;

    if (_cell_has_tally[cell_info])
    {
      _tally_cells.push_back(cell_info);

      if (is_first_tally_cell)
      {
        is_first_tally_cell = false;
        first_tally_cell = cell_info;
        mapped_tally_volume = _cell_to_elem_volume[cell_info];
      }

      if (_check_equal_mapped_tally_volumes)
      {
        Real diff = std::abs(mapped_tally_volume - _cell_to_elem_volume[cell_info]);
        bool absolute_diff = diff > _equal_tally_volume_abs_tol;
        bool relative_diff = diff / mapped_tally_volume > 1e-3;
        if (absolute_diff && relative_diff)
        {
          std::stringstream msg;
          msg << "Detected un-equal mapped tally volumes!\n cell " << printCell(first_tally_cell)
              << " maps to a volume of " << Moose::stringify(_cell_to_elem_volume[first_tally_cell])
              << " (cm3)\n cell " << printCell(cell_info) << " maps to a volume of "
              << Moose::stringify(_cell_to_elem_volume[cell_info])
              << " (cm3).\n\n"
                 "If the tallied cells in your OpenMC model are of identical volumes, this means "
                 "that you can get\n"
                 "distortion of the volumetric tally output. For instance, suppose you have "
                 "two equal-size OpenMC\n"
                 "cells which have the same volume - but each OpenMC cell maps to a MOOSE region "
                 "of different volume\n"
                 "just due to the nature of the centroid mapping scheme. Even if those two tallies "
                 "do actually have the\n"
                 "same value, the volumetric tally will be different because you'll be "
                 "dividing each tally by a\n"
                 "different mapped MOOSE volume.\n\n";

          if (_symmetry)
            msg << "NOTE: You have imposed symmetry, which means that you'll hit this error if any "
                   "of your tally\n"
                   "cells are cut by symmetry planes. If your tally cells would otherwise be the "
                   "same volume if NOT\n"
                   "imposing symmetry, or if your tally cells are not the same volume regardless, "
                   "you need to set\n"
                   "'check_equal_mapped_tally_volumes = false'.";
          else
            msg << "We recommend re-creating the mesh mirror to have an equal volume mapping of "
                   "MOOSE elements to each\n"
                   "OpenMC cell. Or, you can disable this check by setting "
                   "'check_equal_mapped_tally_volumes = false'.";

          mooseError(msg.str());
        }
      }
    }
  }
}

void
OpenMCCellAverageProblem::addLocalTally(const std::vector<std::string> & score, std::vector<openmc::Filter *> & filters)
{
  auto tally = addTally(score, filters, _tally_estimator);
  _local_tally.push_back(tally);
}

std::vector<openmc::Filter *>
OpenMCCellAverageProblem::meshFilter()
{
  std::unique_ptr<openmc::LibMesh> tally_mesh = tallyMesh(_mesh_template_filename);

  _mesh_template = tally_mesh.get();
  _mesh_index = openmc::model::meshes.size();
  openmc::model::meshes.push_back(std::move(tally_mesh));

  std::vector<openmc::Filter *> mesh_filters;

  for (unsigned int i = 0; i < _mesh_translations.size(); ++i)
  {
    const auto & translation = _mesh_translations[i];
    auto meshFilter = dynamic_cast<openmc::MeshFilter *>(openmc::Filter::create("mesh"));
    meshFilter->set_mesh(_mesh_index);
    meshFilter->set_translation({translation(0), translation(1), translation(2)});
    mesh_filters.push_back(meshFilter);
  }

  return mesh_filters;
}

void
OpenMCCellAverageProblem::resetTallies()
{
  // We create the global tally, and THEN the local tally. So we need to delete in
  // reverse order

  auto idx = openmc::model::tallies.begin() + _local_tally_index;
  switch (_tally_type)
  {
    case tally::cell:
    {
      // erase tally
      openmc::model::tallies.erase(idx);

      // erase filter
      auto fidx = openmc::model::tally_filters.begin() + _filter_index;
      openmc::model::tally_filters.erase(fidx);
      break;
    }
    case tally::mesh:
    {
      // erase tallies
      for (int i = _mesh_translations.size() + _local_tally_index - 1; i >= _local_tally_index; --i)
      {
        auto midx = openmc::model::tallies.begin() + i;
        openmc::model::tallies.erase(midx);
      }

      // erase filters
      int fi = _filter_index; // to get signed int for loop to work
      for (int i = _mesh_translations.size() + fi - 1; i >= fi; i--)
      {
        auto fidx = openmc::model::tally_filters.begin() + i;
        openmc::model::tally_filters.erase(fidx);
      }

      // erase mesh
      auto midx = openmc::model::meshes.begin() + _mesh_index;
      openmc::model::meshes.erase(midx);
      break;
    }
    default:
      mooseError("Unhandled TallyTypeEnum in OpenMCCellAverageProblem!");
  }

  if (_needs_global_tally)
  {
    // erase tally
    auto idx = openmc::model::tallies.begin() + _global_tally_index;
    openmc::model::tallies.erase(idx);
  }
}

void
OpenMCCellAverageProblem::initializeTallies()
{
  // add trigger information for k, if present
  openmc::settings::keff_trigger.metric = triggerMetric(_k_trigger);

  // create the global tally for normalization; we make sure to use the
  // same estimator as the local tally
  if (_needs_global_tally)
  {
    _global_tally = openmc::Tally::create();
    _global_tally->set_scores(_tally_score);
    _global_tally->estimator_ = _tally_estimator;

    _global_tally_index = openmc::model::tallies.size() - 1;
    _global_sum_tally.resize(_tally_score.size());
  }

  _local_tally.clear();

  _local_sum_tally.resize(_tally_score.size());
  _local_mean_tally.resize(_tally_score.size());
  _current_tally.resize(_tally_score.size());
  _current_raw_tally.resize(_tally_score.size());
  _current_raw_tally_std_dev.resize(_tally_score.size());
  _previous_tally.resize(_tally_score.size());

  // we have not added the local tally yet, so we do not have the "-1" here. This needs
  // to be before the switch-case statement, because we may add > 1 mesh tally
  _local_tally_index = openmc::model::tallies.size();
  _filter_index = openmc::model::tally_filters.size();

  // create the local tally
  switch (_tally_type)
  {
    case tally::cell:
    {
      _console << "Adding cell tallies to blocks " + Moose::stringify(_tally_blocks) + " for " +
                      Moose::stringify(_tally_cells.size()) + " cells..." << std::endl;

      for (unsigned int i = 0; i < _tally_score.size(); ++i)
      {
        _current_tally[i].resize(1);
        _current_raw_tally[i].resize(1);
        _current_raw_tally_std_dev[i].resize(1);
        _previous_tally[i].resize(1);
      }

      std::vector<openmc::Filter *> filter = {cellInstanceFilter(_tally_cells)};
      addLocalTally(_tally_score, filter);

      break;
    }
    case tally::mesh:
    {
      int n_translations = _mesh_translations.size();

      std::string name = _mesh_template_filename ? *_mesh_template_filename : "the MOOSE [Mesh]";
      _console << "Adding mesh tally based on " + name + " at " +
                      Moose::stringify(n_translations) + " locations"
               << std::endl;

      VariadicTable<int, int, Real, Real, Real, Real> vt({"Mesh template",
                                                          "# Elems",
                                                          "           x",
                                                          "           y",
                                                          "           z",
                                                          "      Volume"});
      vt.setColumnFormat({VariadicTableColumnFormat::AUTO,
                          VariadicTableColumnFormat::AUTO,
                          VariadicTableColumnFormat::SCIENTIFIC,
                          VariadicTableColumnFormat::SCIENTIFIC,
                          VariadicTableColumnFormat::SCIENTIFIC,
                          VariadicTableColumnFormat::SCIENTIFIC});

      for (unsigned int i = 0; i < _tally_score.size(); ++i)
      {
        _current_tally[i].resize(n_translations);
        _current_raw_tally[i].resize(n_translations);
        _current_raw_tally_std_dev[i].resize(n_translations);
        _previous_tally[i].resize(n_translations);
      }

      _mesh_filters.clear();
      auto filters = meshFilter();
      for (const auto & m : filters)
        _mesh_filters.push_back(dynamic_cast<openmc::MeshFilter *>(m));

      for (unsigned int i = 0; i < _mesh_translations.size(); ++i)
      {
        std::vector<openmc::Filter *> filter = {filters[i]};
        addLocalTally(_tally_score, filter);
      }

      if (_verbose)
      {
        for (unsigned int i = 0; i < _mesh_translations.size(); ++i)
        {
          const auto & translation = _mesh_translations[i];

          Real volume = 0.0;
          for (int32_t e = 0; e < _local_tally.at(i)->n_filter_bins(); ++e)
            volume += _mesh_template->volume(e);

          vt.addRow(i,
                    _local_tally.at(i)->n_filter_bins(),
                    translation(0),
                    translation(1),
                    translation(2),
                    volume);
        }

        vt.print(_console);
        _console << std::endl;
      }

      break;
    }
    default:
      mooseError("Unhandled TallyTypeEnum in OpenMCCellAverageProblem!");
  }

  if (_assume_separate_tallies)
    openmc::settings::assume_separate = true;

  // add trigger information, if present. TODO: we could have these triggers be individual
  // for each score later if needed
  for (auto & t : _local_tally)
    for (int score = 0; score < _tally_score.size(); ++score)
      t->triggers_.push_back({triggerMetric(_tally_trigger), _tally_trigger_threshold, score});
}

bool
OpenMCCellAverageProblem::findCell(const Point & point)
{
  _particle.clear();
  _particle.u() = {0., 0., 1.};

  Point pt = transformPointToOpenMC(point);

  _particle.r() = {pt(0), pt(1), pt(2)};
  return !openmc::exhaustive_find_cell(_particle);
}



void
OpenMCCellAverageProblem::addExternalVariables()
{
  _external_vars.resize(_tally_score.size());
  for (unsigned int score = 0; score < _tally_score.size(); ++score)
  {
    auto name = _tally_name[score];
    _tally_var.push_back(addExternalVariable(name) /* all blocks */);

    if (_outputs)
    {
      for (std::size_t i = 0; i < _outputs->size(); ++i)
      {
        std::string n = name + "_" + _output_name[i];
        _external_vars[score].push_back(addExternalVariable(n) /* all blocks */);
      }
    }
  }

  // create the variable that will be used to receive density
  if (_has_fluid_blocks)
  {
    auto number = addExternalVariable("density", &_fluid_block_names);
    for (const auto & s : _fluid_blocks)
      _subdomain_to_density_vars[s] = {number, "density"};
  }

  // create the variable(s) that will be used to receive temperature
  for (const auto & v : _temp_vars_to_blocks)
  {
    auto number = addExternalVariable(v.first, &v.second);

    auto ids = _mesh.getSubdomainIDs(v.second);
    for (const auto & s : ids)
      _subdomain_to_temp_vars[s] = {number, v.first};
  }
}

void
OpenMCCellAverageProblem::externalSolve()
{
  // if using Dufek-Gudowski acceleration and this is not the first iteration, update
  // the number of particles; we put this here so that changing the number of particles
  // doesn't intrude with any other postprocessing routines that happen outside this class's purview
  if (_relaxation == relaxation::dufek_gudowski && !firstSolve())
    dufekGudowskiParticleUpdate();

  OpenMCProblemBase::externalSolve();
}

std::map<OpenMCCellAverageProblem::cellInfo, Real>
OpenMCCellAverageProblem::computeVolumeWeightedCellInput(
  const std::map<SubdomainID, std::pair<unsigned int, std::string>> & var_num,
  const coupling::CouplingFields * phase = nullptr)
{
  const auto sys_number = _aux->number();

  // collect the volume-weighted product across local ranks
  std::vector<Real> volume_product;
  for (const auto & c : _local_cell_to_elem)
  {
    // if a specific phase is passed in, only evaluate for those elements in the phase;
    // in order to have the correct array sizes for gatherCellSum, we set zero values
    // for any cells that aren't in the correct phase, and leave it up to the send...ToOpenMC()
    // routines to properly shield against incorrect phases
    if (phase)
    {
      if (cellCouplingFields(c.first) != *phase)
      {
        volume_product.push_back(0.0 /* dummy value */);
        continue;
      }
    }

    Real product = 0.0;
    for (const auto & e : c.second)
    {
      // we are only accessing local elements here, so no need to check for nullptr
      const auto * elem = _mesh.queryElemPtr(globalElemID(e));
      auto v = var_num.at(elem->subdomain_id()).first;
      auto dof_idx = elem->dof_number(sys_number, v, 0);
      product += (*_serialized_solution)(dof_idx) * elem->volume();
    }

    volume_product.push_back(product);
  }

  std::map<cellInfo, Real> global_volume_product;
  gatherCellSum(volume_product, global_volume_product);

  return global_volume_product;
}

void
OpenMCCellAverageProblem::sendTemperatureToOpenMC()
{
  _console << "Sending temperature to OpenMC cells... " << printNewline();

  double maximum = std::numeric_limits<double>::min();
  double minimum = std::numeric_limits<double>::max();

  // collect the volume-temperature product across local ranks
  std::map<cellInfo, Real> cell_vol_temp = computeVolumeWeightedCellInput(_subdomain_to_temp_vars);

  for (const auto & c : _cell_to_elem)
  {
    auto cell_info = c.first;
    Real average_temp = cell_vol_temp[cell_info] / _cell_to_elem_volume[cell_info];

    minimum = std::min(minimum, average_temp);
    maximum = std::max(maximum, average_temp);

    if (_verbose)
      _console << "Setting cell " << printCell(cell_info) << " [" << _cell_to_n_contained[cell_info]
               << " contained cells] to temperature (K): " << std::setw(4) << average_temp
               << std::endl;

    auto contained_cells = _cell_to_contained_material_cells[cell_info];
    for (const auto & contained : contained_cells)
    {
      for (const auto & instance : contained.second)
        setCellTemperature(contained.first, instance, average_temp, cell_info);
    }
  }

  if (!_verbose)
    _console << "done. Sent cell-averaged min/max (K): " << minimum << ", " << maximum;
  _console << std::endl;
}

OpenMCCellAverageProblem::cellInfo
OpenMCCellAverageProblem::containedMaterialCell(const cellInfo & cell_info)
{
  auto contained_cells = _cell_to_contained_material_cells[cell_info];
  auto instances = contained_cells.begin()->second;
  cellInfo first_cell = {contained_cells.begin()->first, instances[0]};
  return first_cell;
}

void
OpenMCCellAverageProblem::sendDensityToOpenMC()
{
  _console << "Sending density to OpenMC cells... " << printNewline();

  double maximum = std::numeric_limits<double>::min();
  double minimum = std::numeric_limits<double>::max();

  // collect the volume-density product across local ranks
  auto phase = coupling::density_and_temperature;
  std::map<cellInfo, Real> cell_vol_density = computeVolumeWeightedCellInput(_subdomain_to_density_vars, &phase);

  // in case multiple cells are filled by this material, assemble the sum of
  // the rho-V product and V for each of those cells. If _map_density_by_cell
  // is true, then the numerator and denominator are populated from just a single
  // value (no sum)
  std::map<int32_t, Real> numerator;
  std::map<int32_t, Real> denominator;
  for (const auto & c : _cell_to_elem)
  {
    auto cell_info = c.first;

    // dummy values fill cell_vol_density for the non-fluid cells
    if (cellCouplingFields(cell_info) != phase)
      continue;

    auto mat_idx = _cell_to_material[cell_info];

    if (numerator.count(mat_idx))
    {
      numerator[mat_idx] += cell_vol_density[cell_info];
      denominator[mat_idx] += _cell_to_elem_volume[cell_info];
    }
    else
    {
      numerator[mat_idx] = cell_vol_density[cell_info];
      denominator[mat_idx] = _cell_to_elem_volume[cell_info];
    }
  }

  for (const auto & c : _cell_to_elem)
  {
    auto cell_info = c.first;

    // dummy values fill cell_vol_density for the non-fluid cells
    if (cellCouplingFields(cell_info) != phase)
      continue;

    auto mat_idx = _cell_to_material[cell_info];
    Real average_density = numerator[mat_idx] / denominator[mat_idx];

    minimum = std::min(minimum, average_density);
    maximum = std::max(maximum, average_density);

    if (_verbose)
      _console << "Setting cell " << printCell(cell_info) << " to density (kg/m3): " << std::setw(4)
               << average_density << std::endl;

    setCellDensity(average_density, cell_info);
  }

  if (!_verbose)
    _console << "done. Sent cell-averaged min/max (kg/m3): " << minimum << ", " << maximum;
  _console << std::endl;
}

void
OpenMCCellAverageProblem::checkZeroTally(const Real & power_fraction,
                                         const std::string & descriptor,
                                         const unsigned int & score) const
{
  if (_check_zero_tallies && power_fraction < 1e-12)
    mooseError(
        _tally_score[score] + " computed for " + descriptor + " is zero!\n\n" +
        "This may occur if there is no fissile material in this region, if you have very few "
        "particles, "
        "or if you have a geometry "
        "setup error. You can turn off this check by setting 'check_zero_tallies' to false.");
}

Real
OpenMCCellAverageProblem::tallyMultiplier(const unsigned int & score) const
{
  if (!isHeatingScore(_tally_score[score]))
  {
    // we need to get an effective source rate (particles / second) in order to
    // normalize the tally
    Real source = _local_mean_tally[score];
    if (_run_mode == openmc::RunMode::EIGENVALUE)
      source *= *_power / EV_TO_JOULE / _local_mean_tally[_source_rate_index];
    else
      source *= *_source_strength;

    if (_tally_score[score] == "flux")
      return source / _scaling;
    else if (_tally_score[score] == "H3-production")
      return source;
    else
      mooseError("Unhandled tally score enum!");
  }
  else
  {
    // Heating tallies have units of eV / source particle
    if (_run_mode == openmc::RunMode::EIGENVALUE)
      return *_power;
    else
      return *_source_strength * EV_TO_JOULE * _local_mean_tally[score];
  }
}

Real
OpenMCCellAverageProblem::tallyNormalization(const unsigned int & score) const
{
  return _normalize_by_global ? _global_sum_tally[score] : _local_sum_tally[score];
}

template <typename T>
T
OpenMCCellAverageProblem::normalizeLocalTally(const T & tally_result, const unsigned int & score) const
{
  Real comparison = tallyNormalization(score);

  if (std::abs(comparison) < ZERO_TALLY_THRESHOLD)
  {
    // If the value over the whole domain is zero, then the values in the individual bins must be zero.
    // We need to avoid divide-by-zero
    return tally_result * 0.0;
  }
  else
    return tally_result / comparison;
}

void
OpenMCCellAverageProblem::relaxAndNormalizeTally(const int & t, const unsigned int & score)
{
  auto & current = _current_tally[score][t];
  auto & previous = _previous_tally[score][t];
  auto & current_raw = _current_raw_tally[score][t];
  auto & current_raw_std_dev = _current_raw_tally_std_dev[score][t];

  auto tally = _local_tally.at(t);
  auto mean_tally = tallySum(tally, score);
  current_raw = normalizeLocalTally(mean_tally, score);

  auto sum_sq = xt::view(tally->results_, xt::all(), 0, static_cast<int>(openmc::TallyResult::SUM_SQ));
  auto rel_err = relativeError(mean_tally, sum_sq, tally->n_realizations_);
  current_raw_std_dev = rel_err * current_raw;

  // if OpenMC has only run one time, or we don't have relaxation at all,
  // then we don't have a "previous" with which to relax, so we just copy the mean tally in and
  // return
  if (_fixed_point_iteration == 0 || _relaxation == relaxation::none)
  {
    current = current_raw;
    previous = current_raw;
    return;
  }

  // save the current tally (from the previous iteration) into the previous one
  std::copy(current.cbegin(), current.cend(), previous.begin());

  double alpha;
  switch (_relaxation)
  {
    case relaxation::constant:
    {
      alpha = _relaxation_factor;
      break;
    }
    case relaxation::robbins_monro:
    {
      alpha = 1.0 / (_fixed_point_iteration + 1);
      break;
    }
    case relaxation::dufek_gudowski:
    {
      alpha = float(nParticles()) / float(_total_n_particles);
      break;
    }
    default:
      mooseError("Unhandled RelaxationEnum in OpenMCCellAverageProblem!");
  }

  auto relaxed_tally = (1.0 - alpha) * previous + alpha * current_raw;
  std::copy(relaxed_tally.cbegin(), relaxed_tally.cend(), current.begin());
}

void
OpenMCCellAverageProblem::dufekGudowskiParticleUpdate()
{
  int64_t n = (_n_particles_1 + std::sqrt(_n_particles_1 * _n_particles_1 +
                                          4.0 * _n_particles_1 * _total_n_particles)) /
              2.0;
  openmc::settings::n_particles = n;
}

void
OpenMCCellAverageProblem::getTally(const unsigned int & var_num,
  const std::vector<xt::xtensor<double, 1>> & tally, const unsigned int & score, const bool & print_table)
{
  Real sum = 0.0;

  switch (_tally_type)
  {
    case tally::cell:
    {
      sum = getCellTally(var_num, tally, score, print_table);
      break;
    }
    case tally::mesh:
    {
      sum = getMeshTally(var_num, tally, score, print_table);
      break;
    }
    default:
      mooseError("Unhandled TallyTypeEnum in OpenMCCellAverageProblem!");
  }

  if (tallyNormalization(score) > ZERO_TALLY_THRESHOLD)
    if (print_table && _check_tally_sum && std::abs(sum - 1.0) > 1e-6)
      mooseError("Tally normalization process failed for " + _tally_score[score] + " score! Total fraction of " +
                 Moose::stringify(sum) + " does not match 1.0!");
}

Real
OpenMCCellAverageProblem::getCellTally(const unsigned int & var_num,
  const std::vector<xt::xtensor<double, 1>> & tally, const unsigned int & score, const bool & print_table)
{
  VariadicTable<std::string, Real> vt({"Cell", "Fraction of total " + _tally_score[score]});
  vt.setColumnFormat({VariadicTableColumnFormat::AUTO, VariadicTableColumnFormat::SCIENTIFIC});

  Real total = 0.0;

  int i = 0;
  for (const auto & c : _cell_to_elem)
  {
    auto cell_info = c.first;

    // if this cell doesn't have any tallies, skip it
    if (!_cell_has_tally[cell_info])
      continue;

    Real local = tally[0](i++);

    // divide each tally value by the volume that it corresponds to in MOOSE
    // because we will apply it as a volumetric tally
    Real volumetric_power = local * tallyMultiplier(score) / _cell_to_elem_volume[cell_info];
    total += local;

    vt.addRow(printCell(cell_info), local);

    checkZeroTally(local, "cell " + printCell(cell_info), score);
    fillElementalAuxVariable(var_num, c.second, volumetric_power);
  }

  vt.addRow("total", total);

  // do not print a table showing the fractional values for flux, because this tally score
  // itself is not renormalized to preserve some total integral of flux (so the "fraction"
  // is a bit of a misnomer)
  if (_tally_score[score] != "flux")
    if (_verbose && print_table)
      vt.print(_console);

  return total;
}

Real
OpenMCCellAverageProblem::getMeshTally(const unsigned int & var_num,
  const std::vector<xt::xtensor<double, 1>> & tally, const unsigned int & score, const bool & print_table)
{
  VariadicTable<unsigned int, Real> vt({"Mesh", "Fraction of total " + _tally_score[score]});
  vt.setColumnFormat({VariadicTableColumnFormat::AUTO, VariadicTableColumnFormat::SCIENTIFIC});

  Real total = 0.0;

  // TODO: this requires that the mesh exactly correspond to the mesh templates;
  // for cases where they don't match, we'll need to do a nearest-node transfer or something

  unsigned int offset = 0;
  for (unsigned int i = 0; i < _mesh_filters.size(); ++i)
  {
    const auto * filter = _mesh_filters[i];
    Real template_power_fraction = 0.0;

    for (decltype(filter->n_bins()) e = 0; e < filter->n_bins(); ++e)
    {
      Real power_fraction = tally[i](e);

      // divide each tally by the volume that it corresponds to in MOOSE
      // because we will apply it as a volumetric tally (per unit volume).
      // Because we require that the mesh template has units of cm based on the
      // mesh constructors in OpenMC, we need to adjust the division
      Real volumetric_power =
          power_fraction * tallyMultiplier(score) / _mesh_template->volume(e) * _scaling * _scaling * _scaling;
      total += power_fraction;
      template_power_fraction += power_fraction;

      checkZeroTally(power_fraction,
                     "mesh " + Moose::stringify(i) + ", element " + Moose::stringify(e), score);

      std::vector<unsigned int> elem_ids = {offset + e};
      fillElementalAuxVariable(var_num, elem_ids, volumetric_power);
    }

    vt.addRow(i, template_power_fraction);

    offset += filter->n_bins();
  }

  if (_verbose && print_table)
    vt.print(_console);

  return total;
}

void
OpenMCCellAverageProblem::syncSolutions(ExternalProblem::Direction direction)
{
  auto & solution = _aux->solution();

  if (_first_transfer)
    _serialized_solution->init(_aux->sys().n_dofs(), false, SERIAL);

  solution.localize(*_serialized_solution);

  switch (direction)
  {
    case ExternalProblem::Direction::TO_EXTERNAL_APP:
    {
      // re-establish the mapping from the OpenMC cells to the [Mesh], if needed
      if (!_first_transfer && _need_to_reinit_coupling)
      {
        if (_volume_calc)
          _volume_calc->resetVolumeCalculation();

        resetTallies();
        setupProblem();
      }

      if (_first_transfer)
      {
        switch (_initial_condition)
        {
          case coupling::hdf5:
          {
            // if we're reading temperature and density from an existing HDF5 file,
            // we don't need to send anything in to OpenMC, so we can leave.
            importProperties();
            return;
          }
          case coupling::moose:
          {
            // transfer will happen from MOOSE - proceed normally
            break;
          }
          case coupling::xml:
          {
            // if we're just using whatever temperature and density are already in the XML
            // files, we don't need to send anything in to OpenMC, so we can leave.
            std::string incoming_transfer =
                _has_fluid_blocks ? "temperature and density" : "temperature";
            _console << "Skipping " << incoming_transfer << " transfer into OpenMC" << std::endl;
            return;
          }
          default:
            mooseError("Unhandled OpenMCInitialConditionEnum!");
        }
      }

#ifdef ENABLE_DAGMC
      if (_skinner)
      {
        // skin the mesh geometry according to contours in temperature, density, and subdomain
        _skinner->update();

        openmc::model::universe_cell_counts.clear();
        openmc::model::universe_level_counts.clear();

        // Clear nuclides, these will get reset in read_ce_cross_sections
        // Horrible circular logic means that clearing nuclides clears nuclide_map, but
        // which is needed before nuclides gets reset
        std::unordered_map<std::string, int> nuclide_map_copy = openmc::data::nuclide_map;
        openmc::data::nuclides.clear();
        openmc::data::nuclide_map = nuclide_map_copy;

        // Clear existing cell data
        openmc::model::cells.clear();
        openmc::model::cell_map.clear();

        // Clear existing surface data
        openmc::model::surfaces.clear();
        openmc::model::surface_map.clear();

        // Update the OpenMC materials (creating new ones as-needed to support the density binning)
        updateMaterials();

        // regenerate the DAGMC geometry
        reloadDAGMC();

        // we need to then re-establish the data structures that map from OpenMC cells to the [Mesh]
        // (because the cells changed)
        setupProblem();
      }
#else
      // re-establish the mapping from the OpenMC cells to the [Mesh] (because the mesh changed)
      if (_need_to_reinit_coupling)
        setupProblem();
#endif

      // Because we require at least one of fluid_blocks and solid_blocks, we are guaranteed
      // to be setting the temperature of all of the cells in cell_to_elem - only for the density
      // transfer do we need to filter for the fluid cells
      sendTemperatureToOpenMC();

      if (_has_fluid_blocks)
        sendDensityToOpenMC();

      if (_export_properties)
        openmc_properties_export("properties.h5");

      break;
    }
    case ExternalProblem::Direction::FROM_EXTERNAL_APP:
    {
      _console << "Extracting OpenMC tallies... " << printNewline();

      for (unsigned int score = 0; score < _tally_score.size(); ++score)
      {
        // get the total tallies for normalization
        if (_global_tally)
          _global_sum_tally[score] = tallySumAcrossBins({_global_tally}, score);

        _local_sum_tally[score] = tallySumAcrossBins(_local_tally, score);
        _local_mean_tally[score] = tallyMeanAcrossBins(_local_tally, score);

        if (_check_tally_sum)
          checkTallySum(score);

        // Populate the current relaxed and unrelaxed tallies. After this, the _current_tally
        // holds the relaxed tally and _current_raw_tally has the current unrelaxed tally. If
        // no relaxation is used, _current_tally and _current_raw_tally are the same.
        switch (_tally_type)
        {
          case tally::cell:
            relaxAndNormalizeTally(0, score);
            break;
          case tally::mesh:
            for (unsigned int i = 0; i < _mesh_filters.size(); ++i)
              relaxAndNormalizeTally(i, score);
            break;
          default:
            mooseError("Unhandled TallyTypeEnum in OpenMCCellAverageProblem!");
        }

        getTally(_tally_var[score], _current_tally[score], score, true);

        if (_outputs)
        {
          for (std::size_t i = 0; i < _outputs->size(); ++i)
          {
            std::string out = (*_outputs)[i];

            if (out == "unrelaxed_tally_std_dev")
              getTally(_external_vars[score][i], _current_raw_tally_std_dev[score], score, false);
            if (out == "unrelaxed_tally")
              getTally(_external_vars[score][i], _current_raw_tally[score], score, false);
          }
        }
      }

      _console << "done" << std::endl;

      break;
    }
    default:
      mooseError("Unhandled Direction enum in OpenMCCellAverageProblem!");
  }

  _first_transfer = false;
  solution.close();
  _aux->system().update();
}

void
OpenMCCellAverageProblem::checkTallySum(const unsigned int & score) const
{
  if (std::abs(_global_sum_tally[score] - _local_sum_tally[score]) / _global_sum_tally[score] > 1e-6)
  {
    std::stringstream msg;
    msg << _tally_score[score] << " tallies do not match the global " << _tally_score[score] << " tally:\n"
        << " Global value: " << Moose::stringify(_global_sum_tally[score])
        << "\n Tally sum:    " << Moose::stringify(_local_sum_tally[score])
        << "\n Difference:   " << _global_sum_tally[score] - _local_sum_tally[score]
        << "\n\nThis means that the tallies created by Cardinal are missing some hits over the domain.\n"
        << "You can turn off this check by setting 'check_tally_sum' to false.";

    if (_tally_type == tally::mesh)
      msg << "\n\nOr, if your mesh tally doesn't perfectly align with cell boundaries, you could be\n"
             "missing a portion of the scores. To normalize by the total tally (and evade this error),\n"
             "you can set 'normalize_by_global_tally' to false.";

    // Add on extra helpful messages if the domain has a single coordinate level
    // and cell tallies are used
    if (_tally_type == tally::cell && _single_coord_level)
    {
      int n_uncoupled_cells = _n_openmc_cells - _cell_to_elem.size();
      if (n_uncoupled_cells)
        msg << "\n\nYour problem has " + Moose::stringify(n_uncoupled_cells) +
                   " uncoupled OpenMC cells; this warning might be caused by these cells "
                   "contributing\n" +
                   "to the global " << _tally_score[score] << " tally, without being part of the multiphysics "
                   "setup.";
    }

    mooseError(msg.str());
  }
}

void
OpenMCCellAverageProblem::createQRules(QuadratureType type,
                                       Order order,
                                       Order volume_order,
                                       Order face_order,
                                       SubdomainID block,
                                       const bool allow_negative_qweights)
{
  // start copy: Copied from base class's createQRules in order to retain the same default behavior
  if (order == INVALID_ORDER)
  {
    order = getNonlinearSystemBase().getMinQuadratureOrder();
    if (order < getAuxiliarySystem().getMinQuadratureOrder())
      order = getAuxiliarySystem().getMinQuadratureOrder();
  }

  if (volume_order == INVALID_ORDER)
    volume_order = order;

  if (face_order == INVALID_ORDER)
    face_order = order;
  // end copy

  // The approximations made in elem->volume() are only valid for Gauss and Monomial quadratures
  // if they are second order or above
  if (type == Moose::stringToEnum<QuadratureType>("GAUSS"))
    setMinimumVolumeQRules(volume_order, "GAUSS");
  if (type == Moose::stringToEnum<QuadratureType>("MONOMIAL"))
    setMinimumVolumeQRules(volume_order, "MONOMIAL");
  if (type == Moose::stringToEnum<QuadratureType>("GAUSS_LOBATTO"))
    setMinimumVolumeQRules(volume_order, "GAUSS_LOBATTO");

  // Some quadrature rules don't ever seem to give a matching elem->volume() with the MOOSE
  // volume integrations
  if (type == Moose::stringToEnum<QuadratureType>("GRID") ||
      type == Moose::stringToEnum<QuadratureType>("TRAP"))
    mooseError(
        "The ", std::to_string(type), " quadrature set will never match the '_current_elem_volume' used to compute\n"
        "integrals in MOOSE. This means that the tally computed by OpenMC is normalized by\n"
        "a different volume than used for MOOSE volume integrations, such that the specified "
        "'power' or 'source_strength'\n"
        "would not be respected. Please switch to a different quadrature set.");

  FEProblemBase::createQRules(
      type, order, volume_order, face_order, block, allow_negative_qweights);
}

void
OpenMCCellAverageProblem::setMinimumVolumeQRules(Order & volume_order, const std::string & /* type */)
{
  if (volume_order < Moose::stringToEnum<Order>("SECOND"))
    volume_order = SECOND;
}

double
OpenMCCellAverageProblem::cellMappedVolume(const cellInfo & cell_info)
{
  return _cell_to_elem_volume[cell_info];
}

double
OpenMCCellAverageProblem::cellTemperature(const cellInfo & cell_info)
{
  auto material_cell = containedMaterialCell(cell_info);

  double T;
  int err = openmc_cell_get_temperature(material_cell.first, &material_cell.second, &T);
  catchOpenMCError(err, "get temperature of cell " + printCell(cell_info));
  return T;
}

void
OpenMCCellAverageProblem::reloadDAGMC()
{
#ifdef ENABLE_DAGMC
  _dagmc.reset(new moab::DagMC(_skinner->moabPtr()));

  // Set up geometry in DagMC from already-loaded mesh
  _dagmc->load_existing_contents();

  // Initialize acceleration data structures
  _dagmc->init_OBBTree();

  // Get an iterator to the DAGMC universe unique ptr
  auto univ_it = openmc::model::universes.begin() + _dagmc_universe_index;

  // Remove the old universe
  openmc::model::universes.erase(univ_it);

  // Create new DAGMC universe
  openmc::DAGUniverse* dag_univ_ptr = new openmc::DAGUniverse(_dagmc);
  openmc::model::universes.push_back(std::unique_ptr<openmc::DAGUniverse>(dag_univ_ptr));

  _console << "Re-generating OpenMC model with " << openmc::model::cells.size() << " cells... ";

  // Add cells to universes
  openmc::populate_universes();

  // Set the root universe
  openmc::model::root_universe = openmc::find_root_universe();
  openmc::check_dagmc_root_univ();

  // Final geometry setup
  openmc::finalize_geometry();

  // Finalize cross sections
  openmc::finalize_cross_sections();

  // Needed to obtain correct cell instances
  openmc::prepare_distribcell();

  _console << "done" << std::endl;
#endif
}

void
OpenMCCellAverageProblem::updateMaterials()
{
#ifdef ENABLE_DAGMC
  // We currently only re-init the materials one time, because we create one new
  // material for every density bin, even if that density bin doesn't actually
  // appear in the problem. TODO: we could probably reduce memory usage
  // if we only re-generated materials we strictly needed for the model.
  if (!_first_transfer)
    return;

  // only need to create new materials if we have density skinning
  if (_skinner->nDensityBins() == 1)
    return;

  // map from IDs to names (names used by the skinner, not necessarily any internal
  // name in OpenMC, because you're not strictly required to add names for materials
  // with the OpenMC input files)
  std::map<int32_t, std::string> ids_to_names;
  for (const auto & m : openmc::model::material_map)
  {
    auto id = m.first;
    auto idx = m.second;
    if (ids_to_names.count(id))
      mooseError("Internal error: material_map has more than one material with the same ID");

    ids_to_names[id] = materialName(idx);
  }

  // append _0 to all existing material names
  for (const auto & mat : openmc::model::materials)
    mat->set_name(ids_to_names[mat->id()] + "_0");

  // Then, create the copies of each material
  int n_mats = openmc::model::materials.size();
  for (unsigned int n = 0; n < n_mats; ++n)
  {
    auto name = ids_to_names[openmc::model::materials[n]->id()];
    for (unsigned int j = 1; j < _skinner->nDensityBins(); ++j)
    {
      openmc::Material & new_mat = openmc::model::materials[n]->clone();
      new_mat.set_name(name + "_" + std::to_string(j));
    }
  }
#endif
}
#endif
