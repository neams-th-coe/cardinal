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
#include "NonlinearSystemBase.h"
#include "Conversion.h"
#include "VariadicTable.h"
#include "UserErrorChecking.h"

#include "mpi.h"
#include "openmc/capi.h"
#include "openmc/cell.h"
#include "openmc/constants.h"
#include "openmc/error.h"
#include "openmc/particle.h"
#include "openmc/geometry.h"
#include "openmc/geometry_aux.h"
#include "openmc/message_passing.h"
#include "openmc/random_lcg.h"
#include "openmc/settings.h"
#include "openmc/summary.h"
#include "openmc/tallies/trigger.h"
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
                        "Whether to check consistency between the cell-wise tallies "
                        "with a global tally");
  params.addParam<bool>(
      "check_zero_tallies",
      true,
      "Whether to throw an error if any tallies from OpenMC evaluate to zero; "
      "this can be helpful in reducing the number of tallies if you inadvertently add tallies "
      "to a non-fissile region, or for catching geomtery setup errors");
  params.addParam<bool>(
      "skip_first_incoming_transfer",
      false,
      "Whether to skip the very first density and temperature transfer into OpenMC; "
      "this can be used to allow whatever initial condition is set in OpenMC's XML "
      "files to be used in OpenMC's run the first time OpenMC is run");
  params.addParam<MooseEnum>(
      "initial_properties",
      getInitialPropertiesEnum(),
      "Where to read the temperature and density initial conditions for the OpenMC model");

  params.addParam<bool>(
      "export_properties",
      false,
      "Whether to export OpenMC's temperature and density properties after updating "
      "them in the syncSolutions call.");
  params.addRangeCheckedParam<Real>(
      "scaling",
      1.0,
      "scaling > 0.0",
      "Scaling factor to apply to mesh to get to units of centimeters that OpenMC expects; "
      "setting 'scaling = 100.0', for instance, indicates that the mesh is in units of meters");
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

  params.addParam<bool>("fixed_mesh", true,
    "Whether the MooseMesh is unchanging during the simulation (true), or whether there is mesh "
    "movement and/or adaptivity that is changing the mesh in time (false). When the mesh changes "
    "during the simulation, the mapping from OpenMC's cells to the mesh must be re-evaluated after "
    "each OpenMC run.");

  params.addRequiredParam<MooseEnum>(
      "tally_type", getTallyTypeEnum(), "Type of tally to use in OpenMC");
  params.addParam<MooseEnum>(
      "tally_estimator", getTallyEstimatorEnum(), "Type of tally estimator to use in OpenMC");

  MultiMooseEnum scores(
    "heating heating_local kappa_fission fission_q_prompt fission_q_recoverable damage_energy flux");
  params.addParam<MultiMooseEnum>(
      "tally_score", scores, "Score(s) to use in the OpenMC tallies. If not specified, defaults to 'kappa_fission'");
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

  params.addParam<std::vector<std::string>>(
      "temperature_variables",
      "Vector of variable names corresponding to the temperatures that should be assembled into "
      "the 'temp' variable from which OpenMC reads cell temperatures. You may use this if "
      "multiple applications are providing temperature to OpenMC, which need to be collated "
      "together into a single variable");
  params.addParam<std::vector<SubdomainName>>(
      "temperature_blocks", "Blocks corresponding to each of the 'temperature_variables'");

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
  params.addParam<int64_t>("first_iteration_particles",
                           "Number of particles to use for first iteration "
                           "when using Dufek-Gudowski relaxation");

  params.addParam<Point>("symmetry_plane_normal",
                         "Normal that defines a symmetry plane in the OpenMC model");
  params.addParam<Point>("symmetry_axis",
                         "Axis about which to rotate for angle-symmetric OpenMC models");
  params.addRangeCheckedParam<Real>(
      "symmetry_angle",
      "symmetry_angle > 0 & symmetry_angle <= 180",
      "Angle (degrees) from symmetry plane for which OpenMC model is symmetric");
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
    _specified_scaling(params.isParamSetByUser("scaling")),
    _scaling(getParam<Real>("scaling")),
    _run_mode(openmc::settings::run_mode),
    _normalize_by_global(_run_mode == openmc::RunMode::FIXED_SOURCE ? false :
                                      getParam<bool>("normalize_by_global_tally")),
    _fixed_mesh(getParam<bool>("fixed_mesh")),
    _check_tally_sum(isParamValid("check_tally_sum") ? getParam<bool>("check_tally_sum") :
                                                       (_run_mode == openmc::RunMode::FIXED_SOURCE ?
                                                        true : _normalize_by_global)),
    _check_equal_mapped_tally_volumes(getParam<bool>("check_equal_mapped_tally_volumes")),
    _equal_tally_volume_abs_tol(getParam<Real>("equal_tally_volume_abs_tol")),
    _relaxation_factor(getParam<Real>("relaxation_factor")),
    _identical_tally_cell_fills(getParam<bool>("identical_tally_cell_fills")),
    _check_identical_tally_cell_fills(getParam<bool>("check_identical_tally_cell_fills")),
    _assume_separate_tallies(getParam<bool>("assume_separate_tallies")),
    _has_fluid_blocks(params.isParamSetByUser("fluid_blocks")),
    _has_solid_blocks(params.isParamSetByUser("solid_blocks")),
    _needs_global_tally(_check_tally_sum || _normalize_by_global),
    _tally_mesh_from_moose(!isParamValid("mesh_template")),
    _total_n_particles(0),
    _temperature_vars(nullptr),
    _temperature_blocks(nullptr),
    _symmetry(nullptr)
{
  if (_run_mode == openmc::RunMode::FIXED_SOURCE)
    checkUnusedParam(params, "normalize_by_global_tally", "running OpenMC in fixed source mode");

  if (isParamValid("tally_estimator"))
  {
    auto estimator = getParam<MooseEnum>("tally_estimator").getEnum<tally::TallyEstimatorEnum>();
    if (_tally_type == tally::mesh && estimator == tally::tracklength)
      mooseError("Tracklength estimators are currently incompatible with mesh tallies!");

    switch (estimator)
    {
      case tally::tracklength:
        _tally_estimator = openmc::TallyEstimator::TRACKLENGTH;
        break;
      case tally::collision:
        _tally_estimator = openmc::TallyEstimator::COLLISION;
        break;
      case tally::analog:
        _tally_estimator = openmc::TallyEstimator::ANALOG;
        break;
      default:
        mooseError("Unhandled TallyEstimatorEnum in OpenMCCellAverageProblem!");
    }
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

    for (unsigned int i = 0; i < scores.size(); ++i)
    {
      auto score = scores[i];
      std::transform(score.begin(), score.end(), score.begin(),
        [](unsigned char c){ return std::tolower(c); });

      std::replace(score.begin(), score.end(), '_', '-');
      _tally_score.push_back(score);

      if (score == "flux" && _run_mode != openmc::RunMode::FIXED_SOURCE)
        mooseError("The 'flux' tally score is only available when running OpenMC in fixed source mode!\n"
          "Flux renormalization for eigenvalue runs has not been implemented yet.");
    }
  }
  else
    _tally_score = {"kappa-fission"};

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

  std::set<std::string> name(_tally_name.begin(), _tally_name.end());
  std::set<std::string> score(_tally_score.begin(), _tally_score.end());
  if (_tally_name.size() != name.size())
    mooseError("'tally_name' cannot contain duplicate entries!");

  if (_tally_score.size() != score.size())
    mooseError("'tally_score' cannot contain duplicate entries!");

  if (_tally_name.size() != _tally_score.size())
    mooseError("'tally_name' must be the same length as 'tally_score'!");

  if (_tally_type == tally::mesh)
    if (_mesh.getMesh().allow_renumbering() && !_mesh.getMesh().is_replicated())
      mooseError("Mesh tallies currently require 'allow_renumbering = false' to be set in the [Mesh]!");

  if (isParamValid("symmetry_plane_normal"))
  {
    const auto & normal = getParam<Point>("symmetry_plane_normal");
    _symmetry.reset(new SymmetryPointGenerator(normal));

    checkJointParams(params, {"symmetry_axis", "symmetry_angle"}, "specifying angular symmetry");

    if (isParamValid("symmetry_axis"))
    {
      const auto & axis = getParam<Point>("symmetry_axis");
      const auto & angle = getParam<Real>("symmetry_angle");
      _symmetry->initializeAngularSymmetry(axis, angle);
    }
  }
  else
    checkUnusedParam(params, {"symmetry_plane_normal", "symmetry_axis", "symmetry_angle"},
                             "not setting a symmetry plane");

  if (_assume_separate_tallies && _needs_global_tally)
    paramError("assume_separate_tallies",
               "Cannot assume separate tallies when either of 'check_tally_sum' or"
               "'normalize_by_global_tally' is true!");

  if (params.isParamSetByUser("skip_first_incoming_transfer"))
    mooseError("The 'skip_first_incoming_transfer' parameter is deprecated and has been replaced "
               "by the 'initial_properties' parameter!");

  // determine the number of particles set either through XML or the wrapping
  if (_relaxation == relaxation::dufek_gudowski)
  {
    checkUnusedParam(params, "particles", "using Dufek-Gudowski relaxation");
    checkRequiredParam(params, "first_iteration_particles", "using Dufek-Gudowski relaxation");
    openmc::settings::n_particles = getParam<int64_t>("first_iteration_particles");
  }
  else
    checkUnusedParam(params, "first_iteration_particles", "not using Dufek-Gudowski relaxation");

  _n_particles_1 = nParticles();

  // set the parameters needed for tally triggers
  getTallyTriggerParameters(params);

  if (_relaxation != relaxation::constant)
    checkUnusedParam(params, "relaxation_factor", "not using constant relaxation");

  if (!_identical_tally_cell_fills)
    checkUnusedParam(
        params, "check_identical_tally_cell_fills", "'identical_tally_cell_fills' is false");

  checkJointParams(params,
                   {"temperature_variables", "temperature_blocks"},
                   "assembling temperature from multiple variables");

  if (isParamValid("temperature_variables"))
  {
    _temperature_vars = &getParam<std::vector<std::string>>("temperature_variables");
    _temperature_blocks = &getParam<std::vector<SubdomainName>>("temperature_blocks");

    checkEmptyVector(*_temperature_vars, "temperature_variables");
    checkEmptyVector(*_temperature_blocks, "temperature_blocks");

    if (_temperature_vars->size() != _temperature_blocks->size())
      mooseError("'temperature_variables' and 'temperature_blocks' must be the same length!");
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

      if (!_tally_mesh_from_moose)
      {
        _mesh_template_filename = getParam<std::string>("mesh_template");

        if (isParamValid("mesh_translations") && isParamValid("mesh_translations_file"))
          mooseError("Both 'mesh_translations' and 'mesh_translations_file' cannot be specified");
      }
      else
      {
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

  if (!isParamValid("fluid_blocks") && !isParamValid("solid_blocks"))
    mooseError("At least one of 'fluid_blocks' and 'solid_blocks' must be specified to "
               "establish the mapping from MOOSE to OpenMC.");

  readFluidBlocks();
  readSolidBlocks();

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

  setupProblem();

  initializeTallies();

  checkMeshTemplateAndTranslations();
}

void
OpenMCCellAverageProblem::initialSetup()
{
  OpenMCProblemBase::initialSetup();

  if (_adaptivity.isOn() && _fixed_mesh)
    mooseError("When using mesh adaptivity, 'fixed_mesh' must be false!");
}

void
OpenMCCellAverageProblem::setupProblem()
{
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

    if (err)
      mooseError("In attempting to set the maximum number of batches, OpenMC reported:\n\n" +
                 std::string(openmc_err_msg));

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
    paramError(name, "Vector cannot be empty!");
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

    for (unsigned int e = 0; e < filter->n_bins(); ++e)
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

    if (using_single_level && using_lowest_level)
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
                                              std::unordered_set<SubdomainID> & blocks)
{
  std::string param_name = name + "_blocks";

  if (isParamValid(param_name))
  {
    std::vector<SubdomainName> b = getParam<std::vector<SubdomainName>>(param_name);
    checkEmptyVector(b, param_name);

    auto b_ids = _mesh.getSubdomainIDs(b);

    std::copy(b_ids.begin(), b_ids.end(), std::inserter(blocks, blocks.end()));

    const auto & subdomains = _mesh.meshSubdomains();
    for (const auto & b : blocks)
      if (subdomains.find(b) == subdomains.end())
        mooseError("Block " + Moose::stringify(b) + " specified in '" + name +
                   "_blocks' not found in mesh!");
  }
}

const coupling::CouplingFields
OpenMCCellAverageProblem::elemPhase(const Elem * elem) const
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

const coupling::CouplingFields
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

      switch (elemPhase(elem))
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

void
OpenMCCellAverageProblem::checkCellMappedPhase()
{
  VariadicTable<std::string, int, int, int, Real> vt(
      {"Cell", "# Solid", "# Fluid", "# Uncoupled", "Mapped Volume"});

  vt.setColumnFormat({VariadicTableColumnFormat::AUTO,
                      VariadicTableColumnFormat::AUTO,
                      VariadicTableColumnFormat::AUTO,
                      VariadicTableColumnFormat::AUTO,
                      VariadicTableColumnFormat::SCIENTIFIC});

  // whether the entire problem has identified any fluid or solid cells
  bool has_fluid_cells = false;
  bool has_solid_cells = false;

  for (const auto & c : _cell_to_elem)
  {
    auto cell_info = c.first;
    int n_solid = _n_solid[cell_info];
    int n_fluid = _n_fluid[cell_info];
    int n_none = _n_none[cell_info];

    vt.addRow(printCell(cell_info), n_solid, n_fluid, n_none, _cell_to_elem_volume[cell_info]);

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

  if (_verbose)
  {
    _console << "\nMapping of OpenMC cells to MOOSE mesh elements:" << std::endl;
    vt.print(_console);
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
  VariadicTable<std::string, int> vt({"Cell", "Fluid Material"});

  std::set<int32_t> materials;

  for (const auto & c : _cell_to_elem)
  {
    auto cell_info = c.first;

    // skip if the cell isn't fluid
    if (cellCouplingFields(cell_info) != coupling::density_and_temperature)
      continue;

    int fill_type;
    std::vector<int32_t> material_indices = cellFill(cell_info, fill_type);

    // OpenMC checks that for distributed cells, the number of materials either equals 1
    // or the number of distributed cells; therefore, we just need to index based on the cell
    // instance (zero for not-distributed cells, otherwise matches the material index)
    int32_t material_index = material_indices[cell_info.second];
    _cell_to_material[cell_info] = material_index;

    vt.addRow(printCell(cell_info), material_index);

    // check for each material that we haven't already discovered it; if we have, this means we
    // didnt set up the materials correctly
    if (materials.find(material_index) == materials.end())
      materials.insert(material_index);
    else
      mooseError(printMaterial(material_index) + " is present in more than one "
                                                 "fluid cell.\nThis means that your model cannot "
                                                 "independently change the density in cells filled "
                                                 "with this material.");
  }

  if (_verbose && _has_fluid_blocks)
  {
    _console << "\nMaterials in each OpenMC fluid cell:" << std::endl;
    vt.print(_console);
  }
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

  // If there is a single coordinate level, we can print a helpful message if there are uncoupled
  // cells in the domain
  if (_single_coord_level)
  {
    auto n_uncoupled_cells = _n_openmc_cells - _cell_to_elem.size();
    if (n_uncoupled_cells)
      mooseWarning("Skipping multiphysics feedback for " + Moose::stringify(n_uncoupled_cells) +
                   " OpenMC cells!");
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
    auto phase = elemPhase(elem);

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
  auto tally = openmc::Tally::create();
  tally->set_scores(score);
  tally->estimator_ = _tally_estimator;
  tally->set_filters(filters);
  _local_tally.push_back(tally);
}

openmc::Filter *
OpenMCCellAverageProblem::cellInstanceFilter()
{
  auto cell_filter =
      dynamic_cast<openmc::CellInstanceFilter *>(openmc::Filter::create("cellinstance"));

  std::vector<openmc::CellInstance> cells;
  for (const auto & c : _tally_cells)
    cells.push_back(
        {gsl::narrow_cast<gsl::index>(c.first), gsl::narrow_cast<gsl::index>(c.second)});

  cell_filter->set_cell_instances(cells);
  return cell_filter;
}

std::vector<openmc::Filter *>
OpenMCCellAverageProblem::meshFilter()
{
  std::unique_ptr<openmc::LibMesh> tally_mesh;
  if (_tally_mesh_from_moose)
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

    tally_mesh = std::make_unique<openmc::LibMesh>(_mesh.getMesh(), _scaling);
  }
  else
    tally_mesh = std::make_unique<openmc::LibMesh>(_mesh_template_filename, _scaling);

  // by setting the ID to -1, OpenMC will automatically detect the next available ID
  tally_mesh->set_id(-1);
  tally_mesh->output_ = false;
  _mesh_template = tally_mesh.get();

  int32_t mesh_index = openmc::model::meshes.size();
  openmc::model::meshes.push_back(std::move(tally_mesh));

  std::vector<openmc::Filter *> mesh_filters;

  for (unsigned int i = 0; i < _mesh_translations.size(); ++i)
  {
    const auto & translation = _mesh_translations[i];
    auto meshFilter = dynamic_cast<openmc::MeshFilter *>(openmc::Filter::create("mesh"));
    meshFilter->set_mesh(mesh_index);
    meshFilter->set_translation({translation(0), translation(1), translation(2)});
    mesh_filters.push_back(meshFilter);
  }

  return mesh_filters;
}

void
OpenMCCellAverageProblem::initializeTallies()
{
  // add trigger information for k, if present
  switch (_k_trigger)
  {
    case tally::variance:
      openmc::settings::keff_trigger.metric = openmc::TriggerMetric::variance;
      break;
    case tally::std_dev:
      openmc::settings::keff_trigger.metric = openmc::TriggerMetric::standard_deviation;
      break;
    case tally::rel_err:
      openmc::settings::keff_trigger.metric = openmc::TriggerMetric::relative_error;
      break;
    case tally::none:
      break;
    default:
      mooseError("Unhandled TallyTriggerTypeEnum!");
  }

  // create the global tally for normalization
  if (_needs_global_tally)
  {
    _global_tally = openmc::Tally::create();
    _global_tally->set_scores(_tally_score);

    // we want to match the same estimator used for the local tally
    _global_tally->estimator_ = _tally_estimator;

    _global_sum_tally.resize(_tally_score.size());
  }

  _local_sum_tally.resize(_tally_score.size());
  _local_mean_tally.resize(_tally_score.size());
  _current_tally.resize(_tally_score.size());
  _current_raw_tally.resize(_tally_score.size());
  _current_raw_tally_std_dev.resize(_tally_score.size());
  _previous_tally.resize(_tally_score.size());

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

      std::vector<openmc::Filter *> filter = {cellInstanceFilter()};
      addLocalTally(_tally_score, filter);

      break;
    }
    case tally::mesh:
    {
      int n_translations = _mesh_translations.size();

      std::string name = _tally_mesh_from_moose ? "the MOOSE [Mesh]" : _mesh_template_filename;
      _console << "\nAdding mesh tally based on " + name + " at " +
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
  {
    for (int score = 0; score < _tally_score.size(); ++score)
    {
      switch (_tally_trigger)
      {
        case tally::variance:
          t->triggers_.push_back({openmc::TriggerMetric::variance, _tally_trigger_threshold, score});
          break;
        case tally::std_dev:
          t->triggers_.push_back(
              {openmc::TriggerMetric::standard_deviation, _tally_trigger_threshold, score});
          break;
        case tally::rel_err:
          t->triggers_.push_back(
              {openmc::TriggerMetric::relative_error, _tally_trigger_threshold, score});
          break;
        case tally::none:
          break;
        default:
          mooseError("Unhandled TallyTriggerTypeEnum!");
      }
    }
  }

  // if the tally sum check is turned off, write a message informing the user
  if (!_check_tally_sum)
    _console << "Turned OFF tally sum check against global tally" << std::endl;
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
  auto var_params = _factory.getValidParams("MooseVariable");
  var_params.set<MooseEnum>("family") = "MONOMIAL";
  var_params.set<MooseEnum>("order") = "CONSTANT";

  _external_vars.resize(_tally_score.size());
  for (unsigned int score = 0; score < _tally_score.size(); ++score)
  {
    auto name = _tally_name[score];
    checkDuplicateVariableName(name);
    addAuxVariable("MooseVariable", name, var_params);
    _tally_var.push_back(_aux->getFieldVariable<Real>(0, name).number());

    if (_outputs)
    {
      for (std::size_t i = 0; i < _outputs->size(); ++i)
      {
        std::string n = _tally_name[score] + "_" + _output_name[i];
        checkDuplicateVariableName(n);
        addAuxVariable("MooseVariable", n, var_params);
        _external_vars[score].push_back(_aux->getFieldVariable<Real>(0, n).number());
      }
    }
  }

  checkDuplicateVariableName("temp");
  addAuxVariable("MooseVariable", "temp", var_params);
  _temp_var = _aux->getFieldVariable<Real>(0, "temp").number();

  // we need a density variable if we are transferring density into OpenMC
  if (_has_fluid_blocks)
  {
    checkDuplicateVariableName("density");
    addAuxVariable("MooseVariable", "density", var_params);
    _density_var = _aux->getFieldVariable<Real>(0, "density").number();
  }

  // if collating temperature from multiple variables, add the necessary
  // auxiliary kernels and variables to do so
  if (_temperature_vars)
  {
    std::map<std::string, std::vector<SubdomainName>> vars_to_blocks;
    std::set<SubdomainName> blocks;
    for (int i = 0; i < _temperature_vars->size(); ++i)
    {
      auto var = (*_temperature_vars)[i];
      auto block = (*_temperature_blocks)[i];
      vars_to_blocks[var].push_back(block);

      bool already_in_set = blocks.find(block) != blocks.end();
      blocks.insert(block);
      if (already_in_set)
        mooseError("Block " + Moose::stringify(block) +
                   " can only point to a single variable name "
                   "in 'temperature_variables'!");
    }

    // create the variables that will be used to temporarily store temperature;
    // TODO: could allow the order and family to be user-specified
    for (const auto & v : vars_to_blocks)
    {
      auto var_params = _factory.getValidParams("MooseVariable");
      var_params.set<MooseEnum>("family") = "LAGRANGE";
      var_params.set<MooseEnum>("order") = "FIRST";
      var_params.set<std::vector<SubdomainName>>("block") = v.second;
      checkDuplicateVariableName(v.first);
      addAuxVariable("MooseVariable", v.first, var_params);
    }

    for (const auto & v : vars_to_blocks)
    {
      auto params = _factory.getValidParams("SelfAux");
      params.set<AuxVariableName>("variable") = "temp";
      params.set<std::vector<SubdomainName>>("block") = v.second;
      params.set<std::vector<VariableName>>("v") = {v.first};
      params.set<ExecFlagEnum>("execute_on") = {EXEC_INITIAL, EXEC_TIMESTEP_BEGIN};
      addAuxKernel("SelfAux", "collated_temp_from_" + v.first, params);
    }
  }
}

void
OpenMCCellAverageProblem::externalSolve()
{
  bool first_iteration = _fixed_point_iteration < 0;

  // if using Dufek-Gudowski acceleration and this is not the first iteration, update
  // the number of particles; we put this here so that changing the number of particles
  // doesn't intrude with any other postprocessing routines that happen outside this class's purview
  if (_relaxation == relaxation::dufek_gudowski && !first_iteration)
    dufekGudowskiParticleUpdate();

  OpenMCProblemBase::externalSolve();

  _total_n_particles += nParticles();
}

std::map<OpenMCCellAverageProblem::cellInfo, Real>
OpenMCCellAverageProblem::computeVolumeWeightedCellInput(const unsigned int & var_num,
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
      auto dof_idx = elem->dof_number(sys_number, var_num, 0);
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
  std::map<cellInfo, Real> cell_vol_temp = computeVolumeWeightedCellInput(_temp_var);

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
  std::map<cellInfo, Real> cell_vol_density = computeVolumeWeightedCellInput(_density_var, &phase);

  for (const auto & c : _cell_to_elem)
  {
    auto cell_info = c.first;

    // dummy values fill cell_vol_density for the non-fluid cells
    if (cellCouplingFields(cell_info) != phase)
      continue;

    Real average_density = cell_vol_density[cell_info] / _cell_to_elem_volume[cell_info];

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
  if (_tally_score[score] == "flux")
  {
    // Flux tally has units of particle - cm / source particle; we also only get here for fixed source
    return *_source_strength * _local_mean_tally[score] / _scaling;
  }
  else
  {
    // All other tally score options have units of eV / source particle
    if (_run_mode == openmc::RunMode::EIGENVALUE)
      return *_power;
    else
      return *_source_strength * EV_TO_JOULE * _local_mean_tally[score];
  }
}

template <typename T>
T
OpenMCCellAverageProblem::normalizeLocalTally(const T & tally_result, const unsigned int & score) const
{
  Real comparison = _normalize_by_global ? _global_sum_tally[score] : _local_sum_tally[score];

  if (std::abs(comparison) < 1e-12)
  {
    std::string descriptor = _normalize_by_global ? "global" : "local";
    mooseError("Cannot normalize tally by " + descriptor + " sum: ", comparison, " due to divide-by-zero.\n"
      "This means that the ", _tally_score[score], " tally over the entire domain is zero.");
  }

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
      if (!_first_transfer && !_fixed_mesh)
        setupProblem();

      if (_first_transfer)
      {
        switch (_initial_condition)
        {
          case coupling::hdf5:
          {
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
            std::string incoming_transfer =
                _has_fluid_blocks ? "temperature and density" : "temperature";
            _console << "Skipping " << incoming_transfer << " transfer into OpenMC" << std::endl;
            return;
          }
          default:
            mooseError("Unhandled OpenMCInitialConditionEnum!");
        }
      }

      // Because we require at least one of fluid_blocks and solid_blocks, we are guaranteed
      // to be setting the temperature of all of the cells in cell_to_elem - only for the density
      // transfer do we need to filter for the fluid cells
      sendTemperatureToOpenMC();

      if (_export_properties)
        openmc_properties_export("properties.h5");

      if (_has_fluid_blocks)
        sendDensityToOpenMC();

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
  if (std::abs(_global_sum_tally[score] - _local_sum_tally[score]) / _global_sum_tally[score] >
      openmc::FP_REL_PRECISION)
  {
    std::stringstream msg;
    msg << _tally_score[score] << " tallies do not match the global " << _tally_score[score] << " tally:\n"
        << " Global value: " << Moose::stringify(_global_sum_tally[score])
        << "\n Tally sum: " << Moose::stringify(_local_sum_tally[score])
        << "\n\nYou can turn off this check by setting 'check_tally_sum' to false.\n"
           "Or, if you're using mesh tallies that don't perfectly align with cell boundaries,\n"
           "you could be missing a small portion of the tally scores. To normalize by the total\n"
           "tally (and evade this error), you can set 'normalize_by_global_tally' to false.";

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

      // If there are cells in OpenMC's problem that we're not coupling with (and therefore not
      // adding tallies for), it's possible that some of cells we're excluding have fissile
      // material. This could also be caused by us not turning tallies on for the solid and/or fluid
      // that have fissile material (note that this only catches cases where we added blocks in
      // 'fluid_blocks' and 'solid_blocks', but forgot to add them to the 'tally_blocks'
      bool missing_tallies_in_fluid = _has_fluid_blocks && std::includes(_tally_blocks.begin(),
                                                                         _tally_blocks.end(),
                                                                         _fluid_blocks.begin(),
                                                                         _fluid_blocks.end());

      bool missing_tallies_in_solid = _has_solid_blocks && std::includes(_tally_blocks.begin(),
                                                                         _tally_blocks.end(),
                                                                         _solid_blocks.begin(),
                                                                         _solid_blocks.end());

      if (missing_tallies_in_fluid || missing_tallies_in_solid)
      {
        std::string missing_tallies;

        if (missing_tallies_in_fluid && missing_tallies_in_solid)
          missing_tallies = "fluid and solid";
        else if (missing_tallies_in_fluid)
          missing_tallies = "fluid";
        else if (missing_tallies_in_solid)
          missing_tallies = "solid";

        msg << "\n\nYour problem didn't add tallies for the " << missing_tallies
            << "; this warning might be caused by\ntally hits in these regions that "
               "contribute to the global " << _tally_score[score] << " tally, without being\npart of the "
               "multiphysics setup.";
      }
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
OpenMCCellAverageProblem::setMinimumVolumeQRules(Order & volume_order, const std::string & type)
{
  if (volume_order < Moose::stringToEnum<Order>("SECOND"))
  {
    _console << "Increasing " << type << " volume quadrature order from "
             << Moose::stringify(volume_order)
             << " to 2 "
                "to match MOOSE integrations"
             << std::endl;
    volume_order = SECOND;
  }
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

  if (err)
    mooseError("In attempting to get temperature of cell " + printCell(cell_info) +
               ", OpenMC reported:\n\n" + std::string(openmc_err_msg));

  return T;
}

#endif
