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
#include "DelimitedFileReader.h"
#include "TallyBase.h"
#include "AddTallyAction.h"

#include "openmc/constants.h"
#include "openmc/cross_sections.h"
#include "openmc/dagmc.h"
#include "openmc/error.h"
#include "openmc/lattice.h"
#include "openmc/particle.h"
#include "openmc/photon.h"
#include "openmc/message_passing.h"
#include "openmc/nuclide.h"
#include "openmc/random_lcg.h"
#include "openmc/summary.h"
#include "openmc/tallies/trigger.h"
#include "openmc/volume_calc.h"
#include "openmc/universe.h"
#include "xtensor/xarray.hpp"

registerMooseObject("CardinalApp", OpenMCCellAverageProblem);

bool OpenMCCellAverageProblem::_first_transfer = true;
bool OpenMCCellAverageProblem::_printed_initial = false;
bool OpenMCCellAverageProblem::_printed_triso_warning = false;

InputParameters
OpenMCCellAverageProblem::validParams()
{
  InputParameters params = OpenMCProblemBase::validParams();
  params.addParam<bool>("output_cell_mapping",
                        true,
                        "Whether to automatically output the mapping from OpenMC cells to the "
                        "[Mesh], usually for diagnostic purposes");

  params.addParam<bool>("check_tally_sum",
                        "Whether to check consistency between the local tallies "
                        "with a global tally sum");
  params.addParam<MooseEnum>(
      "initial_properties",
      getInitialPropertiesEnum(),
      "Where to read the temperature and density initial conditions for OpenMC");

  params.addParam<bool>("export_properties",
                        false,
                        "Whether to export OpenMC's temperature and density properties to an HDF5 "
                        "file after updating them from MOOSE.");
  params.addParam<bool>(
      "normalize_by_global_tally",
      true,
      "Whether to normalize local tallies by a global tally (true) or else by the sum "
      "of the local tally (false)");
  params.addParam<bool>("assume_separate_tallies",
                        false,
                        "Whether to assume that all tallies added in the XML files or by Cardinal "
                        "are spatially separate. This is a performance optimization");

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

  MooseEnum scores_heat(
    "heating heating_local kappa_fission fission_q_prompt fission_q_recoverable");
  params.addParam<MooseEnum>(
      "source_rate_normalization",
      scores_heat,
      "Score to use for computing the "
      "particle source rate (source/sec) for a certain tallies in "
      "eigenvalue mode. In other words, the "
      "source/sec is computed as (power divided by the global value of this tally)");

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
      "temperature_blocks",
      "Blocks corresponding to each of the 'temperature_variables'. If not specified, "
      "there will be no temperature feedback to OpenMC.");

  params.addParam<std::vector<std::vector<std::string>>>(
      "density_variables",
      "Vector of variable names corresponding to the densities sent into OpenMC. Each entry maps "
      "to the corresponding entry in 'density_blocks.' If not specified, each entry defaults to "
      "'density'");
  params.addParam<std::vector<std::vector<SubdomainName>>>(
      "density_blocks",
      "Blocks corresponding to each of the 'density_variables'. If not specified, "
      "there will be no density feedback to OpenMC.");

  params.addParam<unsigned int>("cell_level",
                                "Coordinate level in OpenMC (across the entire geometry) to use "
                                "for identifying cells");
  params.addParam<unsigned int>(
      "lowest_cell_level",
      "Lowest coordinate level in OpenMC to use for identifying cells. The cell level for coupling "
      "will use the value set with this parameter unless the geometry does not have that many "
      "layers of geometry nesting, in which case the locally lowest depth is used");

  params.addParam<std::vector<SubdomainName>>(
      "identical_cell_fills",
      "Blocks on which the OpenMC cells have identical fill universes; this is an optimization to "
      "speed up initialization for TRISO problems while also reducing memory usage. It is assumed "
      "that any cell which maps to one of these subdomains has exactly the same universe filling "
      "it as all other cells which map to these subdomains. We HIGHLY recommend that the first "
      "time you try using this, that you also set 'check_identical_cell_fills = true' to catch "
      "any possible user errors which would exclude you from using this option safely.");
  params.addParam<bool>(
      "check_identical_cell_fills",
      false,
      "Whether to check that your model does indeed have identical cell fills, allowing "
      "you to set 'identical_cell_fills' to speed up initialization");

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

  params.addParam<UserObjectName>(
      "symmetry_mapper",
      "User object (of type SymmetryPointGenerator) "
      "to map from a symmetric OpenMC model to a full-domain [Mesh]. For example, you can use this "
      "to map from a quarter-symmetric OpenMC model to a whole-domain [Mesh].");

  params.addParam<UserObjectName>(
      "volume_calculation",
      "User object that will perform a stochastic volume calculation to get the OpenMC "
      "cell volumes. This can be used to check that the MOOSE regions to which the cells map are "
      "of approximately the same volume as the true cells.");
  params.addParam<UserObjectName>("skinner", "When using DAGMC geometries, an optional skinner that will "
    "regenerate the OpenMC geometry on-the-fly according to iso-contours of temperature and density");
  return params;
}

OpenMCCellAverageProblem::OpenMCCellAverageProblem(const InputParameters & params)
  : OpenMCProblemBase(params),
    _serialized_solution(NumericVector<Number>::build(_communicator).release()),
    _output_cell_mapping(getParam<bool>("output_cell_mapping")),
    _initial_condition(
        getParam<MooseEnum>("initial_properties").getEnum<coupling::OpenMCInitialCondition>()),
    _relaxation(getParam<MooseEnum>("relaxation").getEnum<relaxation::RelaxationEnum>()),
    _k_trigger(getParam<MooseEnum>("k_trigger").getEnum<trigger::TallyTriggerTypeEnum>()),
    _export_properties(getParam<bool>("export_properties")),
    _normalize_by_global(_run_mode == openmc::RunMode::FIXED_SOURCE
                             ? false
                             : getParam<bool>("normalize_by_global_tally")),
    _need_to_reinit_coupling(!getParam<bool>("fixed_mesh")),
    _check_tally_sum(
        isParamValid("check_tally_sum")
            ? getParam<bool>("check_tally_sum")
            : (_run_mode == openmc::RunMode::FIXED_SOURCE ? true : _normalize_by_global)),
    _relaxation_factor(getParam<Real>("relaxation_factor")),
    _has_identical_cell_fills(params.isParamSetByUser("identical_cell_fills")),
    _check_identical_cell_fills(getParam<bool>("check_identical_cell_fills")),
    _assume_separate_tallies(getParam<bool>("assume_separate_tallies")),
    _map_density_by_cell(getParam<bool>("map_density_by_cell")),
    _specified_density_feedback(params.isParamSetByUser("density_blocks")),
    _specified_temperature_feedback(params.isParamSetByUser("temperature_blocks")),
    _needs_to_map_cells(_specified_density_feedback || _specified_temperature_feedback),
    _needs_global_tally(_check_tally_sum || _normalize_by_global),
    _volume_calc(nullptr),
    _symmetry(nullptr)
{
  // Look through the list of AddTallyActions to see if we have a CellTally. If so, we need to map
  // cells.
  const auto & actions = getMooseApp().actionWarehouse().getActions<AddTallyAction>();
  for (const auto & act : actions)
    _has_cell_tallies = act->getMooseObjectType() == "CellTally" || _has_cell_tallies;
  _needs_to_map_cells = _needs_to_map_cells || _has_cell_tallies;

  if (!_needs_to_map_cells)
    checkUnusedParam(params,
                     "output_cell_mapping",
                     "'temperature_blocks', 'density_blocks', and 'tally_blocks' are empty");

  if (!_specified_temperature_feedback && !_specified_density_feedback)
    checkUnusedParam(
        params, "initial_properties", "'temperature_blocks' and 'density_blocks' are unused");

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

  if (_run_mode != openmc::RunMode::EIGENVALUE && _k_trigger != trigger::none)
    mooseError("Cannot specify a 'k_trigger' for OpenMC runs that are not eigenvalue mode!");

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

  if (!_specified_density_feedback || isParamValid("skinner"))
    checkUnusedParam(params,
                     "map_density_by_cell",
                     "either (i) applying geometry skinning or (ii) 'density_blocks' is empty");

    // OpenMC will throw an error if the geometry contains DAG universes but OpenMC wasn't compiled
    // with DAGMC. So we can assume that if we have a DAGMC geometry, that we will also by this
    // point have DAGMC enabled.
#ifdef ENABLE_DAGMC
  bool has_csg;
  bool has_dag;
  geometryType(has_csg, has_dag);

  if (!has_dag)
    checkUnusedParam(
        params, "skinner", "the OpenMC model does not contain any DagMC universes", true);
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
    for (const auto & universe : openmc::model::universes)
      if (universe->geom_type() == openmc::GeometryType::DAG)
        _dagmc_universe_index = openmc::model::universe_map.at(universe->id_);

    // The newly-generated DAGMC cells could be disjoint in space, in which case
    // it is impossible for us to know with 100% certainty a priori how many materials
    // we would need to create.
    _map_density_by_cell = false;
  }
#else
  checkUnusedParam(params, "skinner", "DAGMC geometries in OpenMC are not enabled in this build of Cardinal");
#endif

  _n_particles_1 = nParticles();

  if (_relaxation != relaxation::constant)
    checkUnusedParam(params, "relaxation_factor", "not using constant relaxation");

  readBlockParameters("identical_cell_fills", _identical_cell_fill_blocks);

  if (!_has_identical_cell_fills)
    checkUnusedParam(
        params, "check_identical_cell_fills", "'identical_cell_fills' is not specified");

  readBlockVariables("temperature", "temp", _temp_vars_to_blocks, _temp_blocks);
  readBlockVariables("density", "density", _density_vars_to_blocks, _density_blocks);

  for (const auto & i : _identical_cell_fill_blocks)
    if (std::find(_density_blocks.begin(), _density_blocks.end(), i) != _density_blocks.end())
      mooseError(
          "Entries in 'identical_cell_fills' cannot be contained in 'density_blocks'; the\n"
          "identical fill universe optimization is not yet implemented for density feedback.");

  if (_needs_to_map_cells)
  {
    if (isParamValid("cell_level") == isParamValid("lowest_cell_level"))
      mooseError("Either 'cell_level' or 'lowest_cell_level' must be specified. You have given "
                 "either both or none.");

    std::string selected_param;
    if (isParamValid("cell_level"))
    {
      _cell_level = getParam<unsigned int>("cell_level");
      selected_param = "cell_level";

      if (_cell_level >= openmc::model::n_coord_levels)
        paramError(selected_param,
                   "Coordinate level for finding cells cannot be greater than total number "
                   "of coordinate levels: " +
                       Moose::stringify(openmc::model::n_coord_levels) + "!");
    }
    else
    {
      _cell_level = getParam<unsigned int>("lowest_cell_level");
      selected_param = "lowest_cell_level";
    }
  }
  else
  {
    checkUnusedParam(params,
                     "cell_level",
                     "'temperature_blocks', 'density_blocks', and 'tally_blocks' are empty");
    checkUnusedParam(params,
                     "lowest_cell_level",
                     "'temperature_blocks', 'density_blocks', and 'tally_blocks' are empty");
  }
}

void
OpenMCCellAverageProblem::readBlockVariables(
    const std::string & param,
    const std::string & default_name,
    std::map<std::string, std::vector<SubdomainName>> & vars_to_specified_blocks,
    std::vector<SubdomainID> & specified_blocks)
{
  std::string b = param + "_blocks";
  std::string v = param + "_variables";

  if (!isParamValid(b))
  {
    checkUnusedParam(parameters(), v, "not setting '" + b + "'");
    return;
  }

  std::vector<std::vector<SubdomainName>> blocks;
  read2DBlockParameters(b, blocks, specified_blocks);

  // now, get the names of those temperature variables
  std::vector<std::vector<std::string>> vars;
  if (isParamValid(v))
  {
    vars = getParam<std::vector<std::vector<std::string>>>(v);

    checkEmptyVector(vars, "'" + v + "");
    for (const auto & t : vars)
      checkEmptyVector(t, "Entries in '" + v + "'");

    if (vars.size() != blocks.size())
      mooseError("'" + v + "' and '" + b + "' must be the same length!\n'" + v + "' is of length " +
                 std::to_string(vars.size()) + " and '" + b + "' is of length " +
                 std::to_string(blocks.size()));

    // TODO: for now, we restrict each set of blocks to map to a single temperature variable
    for (std::size_t i = 0; i < vars.size(); ++i)
      if (vars[i].size() > 1)
        mooseError("Each entry in '" + v + "' must be of length 1. Entry " + std::to_string(i) +
                   " is of length " + std::to_string(vars[i].size()));
  }
  else
  {
    // set a reasonable default, if not specified
    vars.resize(blocks.size(), std::vector<std::string>(1));
    for (std::size_t i = 0; i < blocks.size(); ++i)
      vars[i][0] = default_name;
  }

  for (std::size_t i = 0; i < vars.size(); ++i)
    for (std::size_t j = 0; j < blocks[i].size(); ++j)
      vars_to_specified_blocks[vars[i][0]].push_back(blocks[i][j]);
}

void
OpenMCCellAverageProblem::initialSetup()
{
  OpenMCProblemBase::initialSetup();

  getOpenMCUserObjects();

  if (!_needs_to_map_cells)
    checkUnusedParam(parameters(),
                     "volume_calculation",
                     "'temperature_blocks', 'density_blocks', and 'tally_blocks' are empty");
  else if (isParamValid("volume_calculation"))
  {
    const auto & name = getParam<UserObjectName>("volume_calculation");
    auto * base = &getUserObject<UserObject>(name);

    _volume_calc = dynamic_cast<OpenMCVolumeCalculation *>(base);

    if (!_volume_calc)
      paramError("volume_calculation", "The 'volume_calculation' user object must be of type "
        "OpenMCVolumeCalculation!");
  }

  if (_adaptivity.isOn() && !_need_to_reinit_coupling)
    mooseError("When using mesh adaptivity, 'fixed_mesh' must be false!");

  if (isParamValid("symmetry_mapper"))
  {
    const auto & name = getParam<UserObjectName>("symmetry_mapper");
    auto base = &getUserObject<UserObject>(name);

    _symmetry = dynamic_cast<SymmetryPointGenerator *>(base);

    if (!_symmetry)
      mooseError("The 'symmetry_mapper' user object has to be of type SymmetryPointGenerator!");
  }

  // Get triggers.
  getTallyTriggerParameters(_pars);

  setupProblem();

#ifdef ENABLE_DAGMC
  if (isParamValid("skinner"))
  {
    std::set<SubdomainID> t(_temp_blocks.begin(), _temp_blocks.end());
    std::set<SubdomainID> d(_density_blocks.begin(), _density_blocks.end());

    if (t != d && _specified_density_feedback)
      mooseError("The 'skinner' will apply skinning over the entire domain, and requires that the "
                 "entire problem uses identical settings for feedback. Please update "
                 "'temperature_blocks' and 'density_blocks' to include all blocks.");

    if (_symmetry)
      mooseError("Cannot combine the 'skinner' with 'symmetry_mapper'!\n\nWhen using a skinner, "
        "the [Mesh] must exactly match the underlying OpenMC model, so there is\n"
        "no need to transform spatial coordinates to map between OpenMC and the [Mesh].");

    const auto & name = getParam<UserObjectName>("skinner");
    auto base = &getUserObject<UserObject>(name);

    _skinner = dynamic_cast<MoabSkinner *>(base);

    if (!_skinner)
      paramError("skinner", "The 'skinner' user object must be of type MoabSkinner!");

    if (_skinner->hasDensitySkinning() != _specified_density_feedback)
      mooseError(
          "Detected inconsistent settings for density skinning and 'density_blocks'. If applying "
          "density feedback with 'density_blocks', then you must apply density skinning in the '",
          name,
          "' user object (and vice versa)");

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
    const auto & cell_info = c.first;
    int32_t n_contained = 0;

    for (const auto & cc : containedMaterialCells(cell_info))
      n_contained += cc.second.size();

    _cell_to_n_contained[cell_info] = n_contained;
  }

  subdomainsToMaterials();

  initializeTallies();
}

void
OpenMCCellAverageProblem::getTallyTriggerParameters(const InputParameters & parameters)
{
  // parameters needed for k triggers
  bool has_tally_trigger = false;
  if (_k_trigger != trigger::none)
  {
    checkRequiredParam(parameters, "k_trigger_threshold", "using a k trigger");
    openmc::settings::keff_trigger.threshold = getParam<Real>("k_trigger_threshold");
    has_tally_trigger = true;
  }
  else
    checkUnusedParam(parameters, "k_trigger_threshold", "not using a k trigger");

  // Check to see if any of the local tallies have triggers.
  for (const auto & local_tally : _local_tallies)
    has_tally_trigger = has_tally_trigger || local_tally->hasTrigger();

  if (has_tally_trigger) // at least one trigger
  {
    openmc::settings::trigger_on = true;
    checkRequiredParam(parameters, "max_batches", "using triggers");

    if (_skip_statepoint)
      checkUnusedParam(parameters, "skip_statepoint", "using a trigger");

    int err = openmc_set_n_batches(getParam<unsigned int>("max_batches"),
                                   true /* set the max batches */,
                                   true /* add the last batch for statepoint writing */);
    catchOpenMCError(err, "set the maximum number of batches");

    openmc::settings::trigger_batch_interval = getParam<unsigned int>("batch_interval");
  }
  else
  {
    checkUnusedParam(parameters, "max_batches", "not using triggers");
    checkUnusedParam(parameters, "batch_interval", "not using triggers");

    if (_skip_statepoint)
      openmc::settings::statepoint_batch.clear();
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
OpenMCCellAverageProblem::readBlockParameters(const std::string name,
                                              std::unordered_set<SubdomainID> & blocks)
{
  if (isParamValid(name))
  {
    auto names = getParam<std::vector<SubdomainName>>(name);
    checkEmptyVector(names, "'" + name + "'");

    auto b_ids = _mesh.getSubdomainIDs(names);
    std::copy(b_ids.begin(), b_ids.end(), std::inserter(blocks, blocks.end()));
    checkBlocksInMesh(name, b_ids, names);
  }
}

void
OpenMCCellAverageProblem::checkBlocksInMesh(const std::string name,
                                            const std::vector<SubdomainID> & ids,
                                            const std::vector<SubdomainName> & names) const
{
  const auto & subdomains = _mesh.meshSubdomains();
  for (std::size_t b = 0; b < names.size(); ++b)
    if (subdomains.find(ids[b]) == subdomains.end())
      mooseError("Block '" + names[b] + "' specified in '" + name + "' " + "not found in mesh!");
}

void
OpenMCCellAverageProblem::read2DBlockParameters(const std::string name,
                                                std::vector<std::vector<SubdomainName>> & names,
                                                std::vector<SubdomainID> & flattened_ids)
{
  if (isParamValid(name))
  {
    names = getParam<std::vector<std::vector<SubdomainName>>>(name);

    // check that entire vector is not empty
    checkEmptyVector(names, "'" + name + "'");

    // check that each entry in vector is not empty
    for (const auto & n : names)
      checkEmptyVector(n, "Entries in '" + name + "'");

    // flatten the 2-d set of names into a 1-d vector
    std::vector<SubdomainName> flattened_names;
    for (const auto & slice : names)
      for (const auto & i : slice)
        flattened_names.push_back(i);

    flattened_ids = _mesh.getSubdomainIDs(flattened_names);
    checkBlocksInMesh(name, flattened_ids, flattened_names);

    // should not be any duplicate blocks
    std::set<SubdomainName> n;
    for (const auto & b : flattened_names)
    {
      if (n.count(b))
        mooseError(
            "Subdomains cannot be repeated in '" + name + "'! Subdomain '", b, "' is duplicated.");
      n.insert(b);
    }
  }
}

coupling::CouplingFields
OpenMCCellAverageProblem::elemFeedback(const Elem * elem) const
{
  const auto & id = elem->subdomain_id();
  bool has_density =
      std::find(_density_blocks.begin(), _density_blocks.end(), id) != _density_blocks.end();
  bool has_temp = std::find(_temp_blocks.begin(), _temp_blocks.end(), id) != _temp_blocks.end();

  if (has_density && has_temp)
    return coupling::density_and_temperature;
  else if (!has_density && has_temp)
    return coupling::temperature;
  else if (has_density && !has_temp)
    return coupling::density;
  else
    return coupling::none;
}

void
OpenMCCellAverageProblem::storeElementPhase()
{
  std::set<SubdomainID> excl_temp_blocks;
  std::set<SubdomainID> excl_density_blocks;
  std::set<SubdomainID> intersect;

  std::set<SubdomainID> t(_temp_blocks.begin(), _temp_blocks.end());
  std::set<SubdomainID> d(_density_blocks.begin(), _density_blocks.end());

  std::set_difference(t.begin(),
                      t.end(),
                      d.begin(),
                      d.end(),
                      std::inserter(excl_temp_blocks, excl_temp_blocks.end()));

  std::set_difference(d.begin(),
                      d.end(),
                      t.begin(),
                      t.end(),
                      std::inserter(excl_density_blocks, excl_density_blocks.end()));

  std::set_intersection(
      t.begin(), t.end(), d.begin(), d.end(), std::inserter(intersect, intersect.begin()));

  _n_moose_temp_density_elems = 0;
  for (const auto & s : intersect)
    _n_moose_temp_density_elems += numElemsInSubdomain(s);

  _n_moose_temp_elems = 0;
  for (const auto & s : excl_temp_blocks)
    _n_moose_temp_elems += numElemsInSubdomain(s);

  _n_moose_density_elems = 0;
  for (const auto & s : excl_density_blocks)
    _n_moose_density_elems += numElemsInSubdomain(s);

  _n_moose_none_elems =
      _mesh.nElem() - _n_moose_temp_density_elems - _n_moose_temp_elems - _n_moose_density_elems;
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
OpenMCCellAverageProblem::gatherCellSum(std::vector<T> & local,
                                        std::map<cellInfo, T> & global) const
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
OpenMCCellAverageProblem::cellFeedback(const cellInfo & cell_info) const
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
    return _cell_phase.at(cell_info);
}

void
OpenMCCellAverageProblem::getCellMappedPhase()
{
  std::vector<int> cells_n_temp;
  std::vector<int> cells_n_temp_rho;
  std::vector<int> cells_n_rho;
  std::vector<int> cells_n_none;

  // whether each cell maps to a single phase
  for (const auto & c : _local_cell_to_elem)
  {
    std::vector<int> f(4 /* number of coupling options */, 0);

    // we are looping over local elements, so no need to check for nullptr
    for (const auto & e : c.second)
      f[elemFeedback(_mesh.queryElemPtr(globalElemID(e)))]++;

    cells_n_temp.push_back(f[coupling::temperature]);
    cells_n_temp_rho.push_back(f[coupling::density_and_temperature]);
    cells_n_rho.push_back(f[coupling::density]);
    cells_n_none.push_back(f[coupling::none]);
  }

  gatherCellSum(cells_n_temp, _n_temp);
  gatherCellSum(cells_n_temp_rho, _n_temp_rho);
  gatherCellSum(cells_n_rho, _n_rho);
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

  VariadicTable<std::string, int, int, int, int, std::string, std::string> vt(
      {"Cell", "  T  ", " rho ", "T+rho", "Other", "Mapped Vol", "Actual Vol"});

  bool has_mapping = false;

  std::vector<Real> cv;
  for (const auto & c : _cell_to_elem)
  {
    auto cell_info = c.first;
    int n_temp = _n_temp[cell_info];
    int n_rho = _n_rho[cell_info];
    int n_temp_rho = _n_temp_rho[cell_info];
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
    vt.addRow(printCell(cell_info, true), n_temp, n_rho, n_temp_rho, n_none, map.str(), vol.str());

    // cells can only map to a single type of feedback
    std::vector<bool> conditions = {n_temp_rho > 0, n_temp > 0, n_rho > 0, n_none > 0};
    if (std::count(conditions.begin(), conditions.end(), true) > 1)
    {
      std::stringstream msg;
      std::vector<int> conds = {n_temp, n_rho, n_temp_rho, n_none};
      int size = std::to_string(*std::max_element(conds.begin(), conds.end())).length();
      msg << "Cell " << printCell(cell_info) << " mapped to:\n\n  " << std::setw(size) << n_temp
          << "  elements with temperature feedback\n  " << std::setw(size) << n_rho
          << "  elements with density feedback\n  " << std::setw(size) << n_temp_rho
          << "  elements with both temperature and density feedback\n  " << std::setw(size)
          << n_none
          << "  uncoupled elements\n\n"
             "Each OpenMC cell (ID, instance) pair must map to elements of the same coupling "
             "settings.";
      mooseError(msg.str());
    }

    if (n_temp)
    {
      has_mapping = true;
      _cell_phase[cell_info] = coupling::temperature;
    }
    else if (n_rho)
    {
      has_mapping = true;
      _cell_phase[cell_info] = coupling::density;
    }
    else if (n_temp_rho)
    {
      has_mapping = true;
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

  if (_specified_density_feedback || _specified_temperature_feedback)
    if (!has_mapping)
      mooseError("Feedback was specified using 'temperature_blocks' and/or 'density_blocks', but "
                 "no MOOSE elements mapped to OpenMC cells!");

  if (_verbose && _cell_to_elem.size())
  {
    _console
        << "\n ===================>     MAPPING FROM OPENMC TO MOOSE     <===================\n"
        << std::endl;
    _console << "          T:      # elems providing temperature-only feedback" << std::endl;
    _console << "          rho:    # elems providing density-only feedback" << std::endl;
    _console << "          T+rho:  # elems providing temperature and density feedback" << std::endl;
    _console << "          Other:  # elems which do not provide feedback to OpenMC" << std::endl;
    _console << "                    (but receives a cell tally from OpenMC)" << std::endl;
    _console << "     Mapped Vol:  volume of MOOSE elems each cell maps to" << std::endl;
    _console << "     Actual Vol:  OpenMC cell volume (computed with 'volume_calculation')\n"
             << std::endl;
    vt.print(_console);
  }

  printAuxVariableIO();
  _printed_initial = true;
}

void
OpenMCCellAverageProblem::printAuxVariableIO()
{
  if (_printed_initial)
    return;

  if (!(_specified_density_feedback || _specified_temperature_feedback ||
        _local_tallies.size() > 0))
    return;

  _console << "\n ===================>     AUXVARIABLES FOR OPENMC I/O     <===================\n"
           << std::endl;

  if (_specified_density_feedback || _specified_temperature_feedback)
  {
    _console << "      Subdomain:  subdomain name/ID" << std::endl;
    _console << "    Temperature:  variable OpenMC reads temperature from (empty if no feedback)"
             << std::endl;
    _console << "        Density:  variable OpenMC reads density from (empty if no feedback)\n"
             << std::endl;

    VariadicTable<std::string, std::string, std::string> aux(
        {"Subdomain", "Temperature", "Density"});

    for (const auto & s : _mesh.meshSubdomains())
    {
      std::string temp = _subdomain_to_temp_vars.count(s) ? _subdomain_to_temp_vars[s].second : "";
      std::string rho =
          _subdomain_to_density_vars.count(s) ? _subdomain_to_density_vars[s].second : "";

      if (temp == "" && rho == "")
        continue;

      aux.addRow(subdomainName(s), temp, rho);
    }

    aux.print(_console);
    _console << std::endl;
  }

  if (_local_tallies.size() > 0)
  {
    _console << "    Tally Name:   Cardinal tally object name" << std::endl;
    _console << "    Tally Score:  OpenMC tally score" << std::endl;
    _console << "    AuxVariable:  variable where this score is written\n" << std::endl;

    VariadicTable<std::string, std::string, std::string> tallies(
        {"Tally Name", "Tally Score", "AuxVariable"});
    for (unsigned int i = 0; i < _local_tallies.size(); ++i)
    {
      const auto & scores = _local_tallies[i]->getScores();
      const auto & names = _local_tallies[i]->getAuxVarNames();
      for (unsigned int j = 0; j < scores.size(); ++j)
      {
        if (names.size() == 0)
          continue;

        if (j == 0)
          tallies.addRow(_local_tallies[i]->name(), scores[j], names[j]);
        else
          tallies.addRow("", scores[j], names[j]);
      }
    }

    tallies.print(_console);
  }
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
    for (const auto & s : c.second)
      _cell_to_elem_subdomain[c.first].insert(s);

  // each cell must map to a consistent setting for identical_cell_fills
  // (all of the blocks it maps to must either _all_ be in the identical blocks,
  // or all excluded)
  if (_has_identical_cell_fills)
  {
    for (const auto & c : _cell_to_elem)
    {
      auto cell_info = c.first;
      bool at_least_one_in = false;
      bool at_least_one_out = false;
      SubdomainID in;
      SubdomainID out;
      auto subdomains = _cell_to_elem_subdomain[cell_info];
      for (const auto & s : subdomains)
      {
        if (_identical_cell_fill_blocks.find(s) == _identical_cell_fill_blocks.end())
        {
          at_least_one_out = true;
          out = s;
        }
        else
        {
          at_least_one_in = true;
          in = s;
        }
      }

      if (at_least_one_in && at_least_one_out)
      {
        std::stringstream msg;
        msg << "Cell " << printCell(cell_info)
            << " mapped to inconsistent 'identical_cell_fills' settings.\n"
            << "Subdomain " << in << " is in 'identical_cell_fills', but " << out << " is not.\n\n"
            << "All subdomains to which this cell maps must either ALL be in "
               "'identical_cell_fills' or ALL excluded.";
        mooseError(msg.str());
      }
    }
  }
}

std::set<SubdomainID>
OpenMCCellAverageProblem::coupledSubdomains() const
{
  std::set<SubdomainID> subdomains;
  for (const auto & c : _cell_to_elem)
  {
    const auto & subdomains_spanning_cell = _cell_to_elem_subdomain.at(c.first);
    for (const auto & s : subdomains_spanning_cell)
      subdomains.insert(s);
  }

  return subdomains;
}

void
OpenMCCellAverageProblem::subdomainsToMaterials()
{
  const auto time_start = std::chrono::high_resolution_clock::now();

  TIME_SECTION("subdomainsToMaterials", 3, "Mapping OpenMC Materials to Mesh", true);

  for (const auto & c : _cell_to_elem)
  {
    printTrisoHelp(time_start);

    const auto mats = cellHasIdenticalFill(c.first)
                          ? _first_identical_cell_materials
                          : materialsInCells(containedMaterialCells(c.first));

    for (const auto & s : _cell_to_elem_subdomain.at(c.first))
      for (const auto & m : mats)
        _subdomain_to_material[s].insert(m);
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
    vt.addRow(subdomainName(i), mats);
  }

  if (_cell_to_elem.size())
  {
    _console
        << "\n ===================>  OPENMC SUBDOMAIN MATERIAL MAPPING  <====================\n"
        << std::endl;
    _console << "      Subdomain:  Subdomain name; if unnamed, we show the ID" << std::endl;
    _console << "       Material:  OpenMC material name(s) in this subdomain; if unnamed, we\n"
             << "                  show the ID. If N duplicate material names, we show the\n"
             << "                  number in ( ).\n"
             << std::endl;
    vt.print(_console);
    _console << std::endl;
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

    if (!hasDensityFeedback(cell_info))
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
      mooseError(printMaterial(material_index) +
                 " is present in more than one density feedback cell.\n\nThis means that your "
                 "model cannot independently change the density in cells filled with this "
                 "material. You need to edit your OpenMC model to create additional materials "
                 "unique to each density feedback cell.\n\n"
                 "Or, if you want to apply feedback to a material spanning multiple "
                 "cells, set 'map_density_by_cell' to false.");

    if (!is_material_cell)
      mooseError("Density transfer does not currently support cells filled with universes or lattices!");

    _cell_to_material[cell_info] = material_index;
    vt.addRow(printCell(cell_info), materialID(material_index));
  }

  if (_verbose && _specified_density_feedback)
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
      mooseError(
          printMaterial(f) +
          " is present in more than one OpenMC cell with different "
          "density feedback settings!\nIn other words, this material will have its density changed "
          "by Cardinal (because it is\ncontained in cells which map to the 'density_blocks'), but "
          "this material is also present in\nOTHER OpenMC cells, which will give unintended "
          "behavior "
          "by changing density in ALL parts of the\ndomain containing this material (some of which "
          "have not been coupled via Cardinal).\n\n"
          "Please change your OpenMC model so that unique materials are used in regions which "
          "receive "
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

  if (_cell_to_elem.size() == 0 && _has_cell_tallies)
    mooseError("Did not find any overlap between MOOSE elements and OpenMC cells for "
               "the specified blocks!");

  _console << "\nMapping between " + Moose::stringify(_mesh.nElem()) + " MOOSE elements and " +
                  Moose::stringify(_n_openmc_cells) + " OpenMC cells (on " +
                  Moose::stringify(openmc::model::n_coord_levels) + " coordinate levels):"
           << std::endl;

  VariadicTable<std::string, int, int, int, int> vt(
      {"", "# T Elems", "# rho Elems", "# T+rho Elems", "# Uncoupled Elems"});
  vt.addRow("MOOSE mesh",
            _n_moose_temp_elems,
            _n_moose_density_elems,
            _n_moose_temp_density_elems,
            _n_moose_none_elems);
  vt.addRow("OpenMC cells",
            _n_mapped_temp_elems,
            _n_mapped_density_elems,
            _n_mapped_temp_density_elems,
            _n_mapped_none_elems);
  vt.print(_console);
  _console << std::endl;

  if (_needs_to_map_cells)
  {
    if (_n_moose_temp_elems && (_n_mapped_temp_elems != _n_moose_temp_elems))
      mooseWarning("The [Mesh] has " + Moose::stringify(_n_moose_temp_elems) +
                   " elements providing temperature feedback (the elements in "
                   "'temperature_blocks'), but only " +
                   Moose::stringify(_n_mapped_temp_elems) + " got mapped to OpenMC cells.");

    if (_n_moose_temp_elems && (_n_mapped_density_elems != _n_moose_density_elems))
      mooseWarning("The [Mesh] has " + Moose::stringify(_n_moose_density_elems) +
                   " elements providing density feedback (the elements in "
                   "'density_blocks'), but only " +
                   Moose::stringify(_n_mapped_density_elems) + " got mapped to OpenMC cells.");

    if (_n_moose_temp_density_elems &&
        (_n_mapped_temp_density_elems != _n_moose_temp_density_elems))
      mooseWarning("The [Mesh] has " + Moose::stringify(_n_moose_temp_density_elems) +
                   " elements providing temperature and density feedback (the elements in the "
                   "intersection of 'temperature_blocks' and 'density_blocks'), but only " +
                   Moose::stringify(_n_mapped_temp_density_elems) + " got mapped to OpenMC cells.");

    if (_n_mapped_none_elems && (_specified_temperature_feedback || _specified_density_feedback))
      mooseWarning("Skipping OpenMC multiphysics feedback from " +
                   Moose::stringify(_n_mapped_none_elems) +
                   " [Mesh] elements, which occupy a volume of: " +
                   Moose::stringify(_uncoupled_volume * _scaling * _scaling * _scaling) + " cm3");

    if (_n_openmc_cells < _cell_to_elem.size())
      mooseError("Internal error: _cell_to_elem has length ",
                 _cell_to_elem.size(),
                 " which should\n"
                 "not exceed the number of OpenMC cells, ",
                 _n_openmc_cells);
  }

  // Check that each cell maps to a single phase
  checkCellMappedPhase();
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
OpenMCCellAverageProblem::printTrisoHelp(
    const std::chrono::time_point<std::chrono::high_resolution_clock> & start) const
{
  if (!_printed_triso_warning)
  {
    auto stop = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration<double, std::milli>(stop - start).count() / 1e3;
    if (elapsed > 120.0)
    {
      _printed_triso_warning = true;
      _console << "\nThis is taking a long time. Does your problem have TRISOs/other "
               << "highly heterogeneous geometry?\nIf you are repeating the same TRISO/etc. "
                  "universe many times "
               << "through your OpenMC model, setting\n'identical_cell_fills' will give you a big "
                  "speedup.\n\n"
               << "For more information, consult the Cardinal documentation: "
                  "https://tinyurl.com/54kz9aw8"
               << std::endl;
    }
  }
}

void
OpenMCCellAverageProblem::cacheContainedCells()
{
  TIME_SECTION("cacheContainedCells", 3, "Caching Contained Cells", true);

  bool first_cell = true;
  bool second_cell = false;
  containedCells first_cell_cc;
  containedCells second_cell_cc;
  bool used_cache_shortcut = false;

  int n = -1;
  const auto time_start = std::chrono::high_resolution_clock::now();
  for (const auto & c : _cell_to_elem)
  {
    auto cell_info = c.first;
    Point hint = transformPointToOpenMC(_cell_to_point[cell_info]);

    printTrisoHelp(time_start);

    // default to the normal behavior
    if (!cellHasIdenticalFill(cell_info))
      setContainedCells(cell_info, hint, _cell_to_contained_material_cells);
    else
    {
      used_cache_shortcut = true;
      _n_offset[cell_info] = ++n;

      if (first_cell)
      {
        setContainedCells(cell_info, hint, _cell_to_contained_material_cells);
        first_cell_cc = _cell_to_contained_material_cells[cell_info];
        _first_identical_cell = cell_info;
        _first_identical_cell_materials = materialsInCells(first_cell_cc);
        first_cell = false;
        second_cell = true;
      }
      else if (second_cell)
      {
        setContainedCells(cell_info, hint, _cell_to_contained_material_cells);
        second_cell_cc = _cell_to_contained_material_cells[cell_info];
        second_cell = false;

        // we will check for equivalence in the end mapping later; but here we still need
        // some checks to make sure the structure is compatible
        checkContainedCellsStructure(cell_info, first_cell_cc, second_cell_cc);

        // get the offset for each instance for each contained cell
        for (const auto & f : first_cell_cc)
        {
          const auto id = f.first;
          const auto & instances = f.second;
          const auto & new_instances = second_cell_cc[id];

          std::vector<int32_t> offsets;
          for (unsigned int i = 0; i < instances.size(); ++i)
            offsets.push_back(new_instances[i] - instances[i]);

          _instance_offsets[id] = offsets;
        }
      }
    }
  }

  // only need to check if we were attempting the shortcut
  if (_check_identical_cell_fills)
  {
    TIME_SECTION("verifyCacheContainedCells", 4, "Verifying Cached Contained Cells", true);

    std::map<cellInfo, containedCells> checking_cell_fills;
    for (const auto & c : _cell_to_elem)
      setContainedCells(c.first, transformPointToOpenMC(_cell_to_point[c.first]), checking_cell_fills);

    std::map<cellInfo, containedCells> current_cell_fills;
    for (const auto & c : _cell_to_elem)
      current_cell_fills[c.first] = containedMaterialCells(c.first);

    std::map<cellInfo, containedCells> ordered_reference(checking_cell_fills.begin(),
                                                         checking_cell_fills.end());
    std::map<cellInfo, containedCells> ordered(current_cell_fills.begin(),
                                               current_cell_fills.end());
    compareContainedCells(ordered_reference, ordered);
  }

  if (_has_identical_cell_fills && !used_cache_shortcut)
    mooseWarning("You specified 'identical_cell_fills', but all cells which mapped to these "
                 "subdomains were filled \n"
                 "by a material (as opposed to a universe/lattice), so the 'identical_cell_fills' "
                 "parameter is unused.");
}

void
OpenMCCellAverageProblem::checkContainedCellsStructure(const cellInfo & cell_info,
                                                       containedCells & reference,
                                                       containedCells & compare) const
{
  // make sure the number of keys is the same
  if (reference.size() != compare.size())
    mooseError("The cell caching failed to identify identical number of cell IDs filling cell " +
               printCell(cell_info) + "\nYou must unset 'identical_cell_fills'");

  for (const auto & entry : reference)
  {
    const auto & key = entry.first;

    // check that each key exists
    if (!compare.count(key))
      mooseError("Not all cells contain cell ID " + Moose::stringify(cellID(key)) +
                 ". The offender is: cell " + printCell(cell_info) +
                 ".\nYou must unset 'identical_cell_fills'!");

    // for each int32_t key, compare the std::vector<int32_t> map
    const auto & reference_instances = entry.second;
    const auto & compare_instances = compare[key];

    // they should have the same number of instances
    if (reference_instances.size() != compare_instances.size())
      mooseError("The cell caching should have identified " +
                 Moose::stringify(reference_instances.size()) + "cell instances in cell ID " +
                 Moose::stringify(cellID(key)) + ", but instead found " +
                 Moose::stringify(compare_instances.size()) +
                 "\nYou must unset 'identical_cell_fills'");
  }
}

void
OpenMCCellAverageProblem::compareContainedCells(std::map<cellInfo, containedCells> & reference,
                                                std::map<cellInfo, containedCells> & compare) const
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
        mooseError(
            "The cell caching failed to get correct instances for material cell ID " +
            Moose::stringify(cellID(nested_entry.first)) + " within cell " + printCell(cell_info) +
            ". You must unset 'identical_cell_fills'!" + "\n\nThis error might appear if:\n" +
            " - There is a mismatch between your OpenMC model and the [Mesh]\n"
            " - There are additional OpenMC cells filled with this repeatable universe/lattice, "
            "but which are not mapping to the blocks in 'identical_cell_fills'");
    }
  }
}

unsigned int
OpenMCCellAverageProblem::getCellLevel(const Point & c) const
{
  unsigned int level = _cell_level;
  if (_cell_level > _particle.n_coord() - 1)
  {
    if (isParamValid("lowest_cell_level"))
      level = _particle.n_coord() - 1;
    else
      mooseError("Requested coordinate level of " + Moose::stringify(_cell_level) +
                 " exceeds number of nested coordinate levels at " + printPoint(c) + ": " +
                 Moose::stringify(_particle.n_coord()));
  }

  return level;
}

void
OpenMCCellAverageProblem::mapElemsToCells()
{
  // reset counters, flags
  _n_mapped_temp_elems = 0;
  _n_mapped_density_elems = 0;
  _n_mapped_temp_density_elems = 0;
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

    auto id = elem->subdomain_id();
    const Point & c = elem->vertex_average();
    Real element_volume = elem->volume();

    // find the OpenMC cell at the location 'c' (if any)
    bool error = findCell(c);

    // if we didn't find an OpenMC cell here, then we certainly have an uncoupled region
    if (error)
    {
      _uncoupled_volume += element_volume;
      _n_mapped_none_elems++;
      continue;
    }

    // next, see what type of data is to be sent into OpenMC (to further classify
    // the type of couling)
    auto phase = elemFeedback(elem);

    bool requires_mapping = phase != coupling::none || _contains_cell_tally;

    // get the level in the OpenMC model to fetch mapped cell information. For
    // uncoupled regions, we know we will be successful in finding a cell (because
    // we already screened out uncoupled cells), and the id and instance are unused
    // (so we can just set zero).
    auto level = requires_mapping ? getCellLevel(c) : 0;

    // ensure the mapped cell isn't in a unvierse being used as the "outer"
    // universe of a lattice in the OpenMC model
    if (requires_mapping)
      latticeOuterCheck(c, level);

    switch (phase)
    {
      case coupling::density_and_temperature:
      {
        _n_mapped_temp_density_elems++;
        break;
      }
      case coupling::temperature:
      {
        _n_mapped_temp_elems++;
        break;
      }
      case coupling::density:
      {
        _n_mapped_density_elems++;
        break;
      }
      case coupling::none:
      {
        _uncoupled_volume += element_volume;
        _n_mapped_none_elems++;
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

    // store the map of cells to elements that will be coupled via feedback or a tally
    if (requires_mapping)
      _cell_to_elem[cell_info].push_back(local_elem);
  }

  _communicator.sum(_n_mapped_temp_elems);
  _communicator.sum(_n_mapped_temp_density_elems);
  _communicator.sum(_n_mapped_density_elems);
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
OpenMCCellAverageProblem::resetTallies()
{
  if (_local_tallies.size() == 0 && !_needs_global_tally)
    return;

  // We initialize tallies by forward iterating this vector. We need to delete them in reverse.
  for (int i = _local_tallies.size() - 1; i >= 0; --i)
    _local_tallies[i]->resetTally();

  // erase global tallies
  if (_needs_global_tally)
  {
    for (int i = _global_tally_index + _global_tally_scores.size() - 1; i >= 0; --i)
    {
      auto idx = openmc::model::tallies.begin() + _global_tally_index + i;
      openmc::model::tallies.erase(idx);
    }
  }
}

void
OpenMCCellAverageProblem::initializeTallies()
{
  // add trigger information for k, if present
  openmc::settings::keff_trigger.metric = triggerMetric(_k_trigger);

  if (_local_tallies.size() == 0 && !_needs_global_tally)
    return;

  // create the global tally for normalization; we make sure to use the
  // same estimator as the local tally
  if (_needs_global_tally)
  {
    _global_tally_index = openmc::model::tallies.size();

    _global_tallies.clear();
    for (unsigned int i = 0; i < _global_tally_scores.size(); ++i)
    {
      _global_tallies.push_back(openmc::Tally::create());
      _global_tallies[i]->set_scores(_global_tally_scores[i]);
      _global_tallies[i]->estimator_ = _global_tally_estimators[i];
    }

    _global_sum_tally.clear();
    _global_sum_tally.resize(_all_tally_scores.size(), 0.0);
  }

  // Initialize all of the [Tallies].
  for (auto & local_tally : _local_tallies)
    local_tally->initializeTally();
}

void
OpenMCCellAverageProblem::latticeOuterError(const Point & c, int level) const
{
  const auto & cell = openmc::model::cells[_particle.coord(level).cell];
  std::stringstream msg;
  msg << "The point " << c << " mapped to cell " << cell->id_
      << " in the OpenMC model is inside a universe "
         "used as the 'outer' universe of a lattice. "
         "All cells used for mapping in lattices must be explicitly set "
         "on the 'universes' attribute of lattice objects. "
      << "If you want to obtain feedback or cell tallies here, you "
         "will need to widen your lattice to have universes covering all of the space you "
         "want feedback or cell tallies.\n\nFor more information, see: "
         "https://github.com/openmc-dev/openmc/issues/551.";
  mooseError(msg.str());
}

void
OpenMCCellAverageProblem::latticeOuterCheck(const Point & c, int level) const
{
  for (int i = 0; i <= level; ++i)
  {
    const auto & coord = _particle.coord(i);

    // if there is no lattice at this level, move on
    if (coord.lattice == openmc::C_NONE)
      continue;

    const auto & lat = openmc::model::lattices[coord.lattice];

    // if the lattice's outer universe isn't set, move on
    if (lat->outer_ == openmc::NO_OUTER_UNIVERSE)
      continue;

    if (coord.universe != lat->outer_)
      continue;

    // move on if the lattice indices are valid (position is in the set of explicitly defined
    // universes)
    if (lat->are_valid_indices(coord.lattice_i))
      continue;

    // if we get here, the mapping is occurring in a universe that is not explicitly defined in the
    // lattice
    latticeOuterError(c, level);
  }
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
  // We need to validate tallies here to we can add scores that may be missing.
  validateLocalTallies();

  // Add all of the auxvariables in which the [Tallies] block will store results.
  unsigned int previous_valid_name_index = 0;
  for (unsigned int i = 0; i < _local_tallies.size(); ++i)
  {
    _tally_var_ids.emplace_back();

    // We use this to check if a sequence of added tallies corresponds to a single translated mesh.
    // If the number of names reported in getAuxVarNames is zero, the tally must store it's results
    // in the variables added by the first mesh tally in the sequence.
    bool is_instanced = _local_tallies[i]->getAuxVarNames().size() == 0;
    previous_valid_name_index = !is_instanced ? i : previous_valid_name_index;

    const auto & names = _local_tallies[previous_valid_name_index]->getAuxVarNames();

    _tally_ext_var_ids.emplace_back();
    if (_local_tallies[i]->hasOutputs())
      _tally_ext_var_ids[i].resize(_local_tallies[i]->getOutputs().size());

    for (unsigned int j = 0; j < names.size(); ++j)
    {
      if (is_instanced)
        _tally_var_ids[i].push_back(
            _tally_var_ids[previous_valid_name_index][j]); // Use variables from first in sequence.
      else
        _tally_var_ids[i].push_back(addExternalVariable(names[j]));

      if (_local_tallies[i]->hasOutputs())
      {
        const auto & outs = _local_tallies[i]->getOutputs();
        for (std::size_t k = 0; k < outs.size(); ++k)
        {
          std::string n = names[j] + "_" + outs[k];
          if (is_instanced)
            _tally_ext_var_ids[i][k].push_back(
                _tally_ext_var_ids[previous_valid_name_index][k]
                                  [j]); // Use variables from first in sequence.
          else
            _tally_ext_var_ids[i][k].push_back(addExternalVariable(n));
        }
      }
    }
  }

  // create the variable(s) that will be used to receive density
  for (const auto & v : _density_vars_to_blocks)
  {
    auto number = addExternalVariable(v.first, &v.second);

    auto ids = _mesh.getSubdomainIDs(v.second);
    for (const auto & s : ids)
      _subdomain_to_density_vars[s] = {number, v.first};
  }

  // create the variable(s) that will be used to receive temperature
  for (const auto & v : _temp_vars_to_blocks)
  {
    auto number = addExternalVariable(v.first, &v.second);

    auto ids = _mesh.getSubdomainIDs(v.second);
    for (const auto & s : ids)
      _subdomain_to_temp_vars[s] = {number, v.first};
  }

  if (_output_cell_mapping && _needs_to_map_cells)
  {
    std::string auxk_type = "CellIDAux";
    InputParameters params = _factory.getValidParams(auxk_type);
    addExternalVariable("cell_id");
    params.set<AuxVariableName>("variable") = "cell_id";
    addAuxKernel(auxk_type, "cell_id", params);

    auxk_type = "CellInstanceAux";
    params = _factory.getValidParams(auxk_type);
    addExternalVariable("cell_instance");
    params.set<AuxVariableName>("variable") = "cell_instance";
    addAuxKernel(auxk_type, "cell_instance", params);
  }
  else
    _console << "Skipping output of 'cell_id' and 'cell_instance' because 'temperature_blocks', "
                "'density_blocks', and 'tally_blocks' are all empty"
             << std::endl;
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
    const std::vector<coupling::CouplingFields> * phase = nullptr) const
{
  const auto & sys_number = _aux->number();

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
      if (std::find(phase->begin(), phase->end(), cellFeedback(c.first)) == phase->end())
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
OpenMCCellAverageProblem::sendTemperatureToOpenMC() const
{
  if (!_specified_temperature_feedback)
    return;

  _console << "Sending temperature to OpenMC cells... " << printNewline();

  double maximum = std::numeric_limits<double>::min();
  double minimum = std::numeric_limits<double>::max();

  // collect the volume-temperature product across local ranks
  std::vector<coupling::CouplingFields> phase = {coupling::temperature,
                                                 coupling::density_and_temperature};
  std::map<cellInfo, Real> cell_vol_temp =
      computeVolumeWeightedCellInput(_subdomain_to_temp_vars, &phase);

  std::unordered_set<cellInfo> cells_already_set;

  for (const auto & c : _cell_to_elem)
  {
    auto cell_info = c.first;
    if (!hasTemperatureFeedback(cell_info))
      continue;

    Real average_temp = cell_vol_temp.at(cell_info) / _cell_to_elem_volume.at(cell_info);

    minimum = std::min(minimum, average_temp);
    maximum = std::max(maximum, average_temp);

    if (_verbose)
      _console << "Setting cell " << printCell(cell_info) << " ["
               << _cell_to_n_contained.at(cell_info)
               << " contained cells] to temperature (K): " << std::setw(4) << average_temp
               << std::endl;

    containedCells contained_cells = containedMaterialCells(cell_info);

    for (const auto & contained : contained_cells)
      for (const auto & instance : contained.second)
      {
        cellInfo ci = {contained.first, instance};
        if (cells_already_set.count(ci))
        {
          double T;
          openmc_cell_get_temperature(ci.first, &ci.second, &T);

          mooseError("Cell " + std::to_string(cellID(contained.first)) + ", instance " +
                     std::to_string(instance) +
                     " has already had its temperature set by Cardinal to " + std::to_string(T) +
                     "! This indicates a problem with how you have built your geometry, because "
                     "this cell is trying to receive a distribution of temperatures in space, but "
                     "each successive set-temperature operation is only overwriting the previous "
                     "value.\n\nThis error most often appears when you are filling a LATTICE into "
                     "multiple cells. One fix is to first place that lattice into a universe, and "
                     "then fill that UNIVERSE into multiple cells.");
        }

        cells_already_set.insert(ci);
        setCellTemperature(contained.first, instance, average_temp, cell_info);
      }
  }

  if (!_verbose)
    _console << "done. Sent cell-averaged min/max (K): " << minimum << ", " << maximum;
  _console << std::endl;
}

OpenMCCellAverageProblem::cellInfo
OpenMCCellAverageProblem::firstContainedMaterialCell(const cellInfo & cell_info) const
{
  const auto & contained_cells = containedMaterialCells(cell_info);
  const auto & instances = contained_cells.begin()->second;
  cellInfo first_cell = {contained_cells.begin()->first, instances[0]};
  return first_cell;
}

void
OpenMCCellAverageProblem::sendDensityToOpenMC() const
{
  if (!_specified_density_feedback)
    return;

  _console << "Sending density to OpenMC cells... " << printNewline();

  double maximum = std::numeric_limits<double>::min();
  double minimum = std::numeric_limits<double>::max();

  // collect the volume-density product across local ranks
  std::vector<coupling::CouplingFields> phase = {coupling::density,
                                                 coupling::density_and_temperature};
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

    if (!hasDensityFeedback(cell_info))
      continue;

    auto mat_idx = _cell_to_material.at(cell_info);

    if (numerator.count(mat_idx))
    {
      numerator[mat_idx] += cell_vol_density.at(cell_info);
      denominator[mat_idx] += _cell_to_elem_volume.at(cell_info);
    }
    else
    {
      numerator[mat_idx] = cell_vol_density.at(cell_info);
      denominator[mat_idx] = _cell_to_elem_volume.at(cell_info);
    }
  }

  for (const auto & c : _cell_to_elem)
  {
    auto cell_info = c.first;

    if (!hasDensityFeedback(cell_info))
      continue;

    auto mat_idx = _cell_to_material.at(cell_info);
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

Real
OpenMCCellAverageProblem::tallyMultiplier(unsigned int global_score) const
{
  if (!isHeatingScore(_all_tally_scores[global_score]))
  {
    // we need to get an effective source rate (particles / second) in order to
    // normalize the tally
    Real source = _local_mean_tally[global_score];
    if (_run_mode == openmc::RunMode::EIGENVALUE)
      source *= *_power / EV_TO_JOULE / _local_mean_tally[_source_rate_index];
    else
      source *= *_source_strength;

    if (_all_tally_scores[global_score] == "flux")
      return source / _scaling;
    else if (_all_tally_scores[global_score] == "H3-production")
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
      return *_source_strength * EV_TO_JOULE * _local_mean_tally[global_score];
  }
}

Real
OpenMCCellAverageProblem::tallyNormalization(unsigned int global_score) const
{
  return _normalize_by_global ? _global_sum_tally[global_score] : _local_sum_tally[global_score];
}

void
OpenMCCellAverageProblem::relaxAndNormalizeTally(unsigned int global_score,
                                                 unsigned int local_score,
                                                 std::shared_ptr<TallyBase> local_tally)
{
  Real comparison = tallyNormalization(global_score);

  Real alpha;
  switch (_relaxation)
  {
    case relaxation::none:
    {
      alpha = 1.0;
      break;
    }
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

  local_tally->relaxAndNormalizeTally(local_score, alpha, comparison);
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
OpenMCCellAverageProblem::checkNormalization(const Real & sum, unsigned int global_score) const
{
  if (tallyNormalization(global_score) > ZERO_TALLY_THRESHOLD)
    if (_check_tally_sum && std::abs(sum - 1.0) > 1e-6)
      mooseError("Tally normalization process failed for " + _all_tally_scores[global_score] +
                 " score! Total fraction of " + Moose::stringify(sum) + " does not match 1.0!");
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

      // Change nuclide composition of material; we put this here so that we can still then change
      // the _overall_ density (like due to thermal expansion, which does not change the relative
      // amounts of the different nuclides)
      sendNuclideDensitiesToOpenMC();

      sendTallyNuclidesToOpenMC();

      if (_first_transfer && (_specified_temperature_feedback || _specified_density_feedback))
      {
        std::string incoming_transfer =
            _specified_density_feedback ? "temperature and density" : "temperature";

        switch (_initial_condition)
        {
          case coupling::hdf5:
          {
            // if we're reading temperature and density from an existing HDF5 file,
            // we don't need to send anything in to OpenMC, so we can leave.
            importProperties();
            _console << "Skipping " << incoming_transfer
                     << " transfer into OpenMC because 'initial_properties = hdf5'" << std::endl;
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
            _console << "Skipping " << incoming_transfer
                     << " transfer into OpenMC because 'initial_properties = xml'" << std::endl;
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

        // Clear nuclides and elements, these will get reset in read_ce_cross_sections
        // Horrible circular logic means that clearing nuclides clears nuclide_map, but
        // which is needed before nuclides gets reset (similar for elements)
        std::unordered_map<std::string, int> nuclide_map_copy = openmc::data::nuclide_map;
        openmc::data::nuclides.clear();
        openmc::data::nuclide_map = nuclide_map_copy;

        std::unordered_map<std::string, int> element_map_copy = openmc::data::element_map;
        openmc::data::elements.clear();
        openmc::data::element_map = element_map_copy;

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

      sendDensityToOpenMC();

      if (_export_properties)
        openmc_properties_export("properties.h5");

      break;
    }
    case ExternalProblem::Direction::FROM_EXTERNAL_APP:
    {
      _console << "Extracting OpenMC tallies...";

      if (_local_tallies.size() == 0 && _global_tallies.size() == 0)
        break;

      // Get the total tallies for normalization
      if (_global_tallies.size() > 0)
      {
        for (unsigned int global_score = 0; global_score < _all_tally_scores.size(); ++global_score)
        {
          for (unsigned int i = 0; i < _global_tallies.size(); ++i)
          {
            auto loc = std::find(_global_tally_scores[i].begin(),
                                 _global_tally_scores[i].end(),
                                 _all_tally_scores[global_score]);
            if (loc == _global_tally_scores[i].end())
              continue;

            auto index = loc - _global_tally_scores[i].begin();
            _global_sum_tally[global_score] = tallySumAcrossBins({_global_tallies[i]}, index);
          }
        }
      }

      // Loop over all of the tallies and calculate their sums and averages.
      for (auto & local_tally : _local_tallies)
        local_tally->computeSumAndMean();

      // Accumulate the sums and means for every score.
      _local_sum_tally.clear();
      _local_sum_tally.resize(_all_tally_scores.size(), 0.0);
      _local_mean_tally.clear();
      _local_mean_tally.resize(_all_tally_scores.size(), 0.0);
      for (unsigned int i = 0; i < _local_tallies.size(); ++i)
      {
        for (unsigned int global_score = 0; global_score < _all_tally_scores.size(); ++global_score)
        {
          const auto & tally_name = _all_tally_scores[global_score];
          if (_local_tally_score_map[i].count(tally_name) ==
              0) // If the local tally doesn't have this score, skip it.
            continue;

          auto local_score = _local_tally_score_map[i].at(_all_tally_scores[global_score]);
          _local_sum_tally[global_score] += _local_tallies[i]->getSum(local_score);
          _local_mean_tally[global_score] += _local_tallies[i]->getMean(local_score);
        }
      }

      if (_check_tally_sum)
        for (unsigned int global_score = 0; global_score < _all_tally_scores.size(); ++global_score)
          checkTallySum(global_score);

      // Loop over the tallies to relax and normalize their results score by score. Then, store the
      // results.
      std::vector<Real> sums;
      sums.resize(_all_tally_scores.size(), 0.0);
      for (unsigned int i = 0; i < _local_tallies.size(); ++i)
      {
        for (unsigned int global_score = 0; global_score < _all_tally_scores.size(); ++global_score)
        {
          const auto & tally_name = _all_tally_scores[global_score];
          if (_local_tally_score_map[i].count(tally_name) ==
              0) // If the local tally doesn't have this score, skip it.
            continue;

          auto local_score = _local_tally_score_map[i].at(tally_name);
          relaxAndNormalizeTally(global_score, local_score, _local_tallies[i]);

          // Store the tally results.
          sums[global_score] += _local_tallies[i]->storeResults(
              _tally_var_ids[i], local_score, global_score, "relaxed");

          // Store additional tally outputs.
          if (_local_tallies[i]->hasOutputs())
          {
            const auto & outs = _local_tallies[i]->getOutputs();
            for (unsigned int j = 0; j < outs.size(); ++j)
              _local_tallies[i]->storeResults(
                  _tally_ext_var_ids[i][j], local_score, global_score, outs[j]);
          }
        }
      }

      // Check the normalization.
      for (unsigned int global_score = 0; global_score < _all_tally_scores.size(); ++global_score)
        checkNormalization(sums[global_score], global_score);

      _console << " done" << std::endl;

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
    msg << _all_tally_scores[score] << " tallies do not match the global "
        << _all_tally_scores[score] << " tally:\n"
        << " Global value: " << Moose::stringify(_global_sum_tally[score])
        << "\n Tally sum:    " << Moose::stringify(_local_sum_tally[score])
        << "\n Difference:   " << _global_sum_tally[score] - _local_sum_tally[score]
        << "\n\nThis means that the tallies created by Cardinal are missing some hits over the "
           "domain.\n"
        << "You can turn off this check by setting 'check_tally_sum' to false.";

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
    order = getNonlinearSystemBase(0).getMinQuadratureOrder();
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
OpenMCCellAverageProblem::cellMappedVolume(const cellInfo & cell_info) const
{
  return _cell_to_elem_volume.at(cell_info);
}

double
OpenMCCellAverageProblem::cellTemperature(const cellInfo & cell_info) const
{
  auto material_cell = firstContainedMaterialCell(cell_info);

  double T;
  int err = openmc_cell_get_temperature(material_cell.first, &material_cell.second, &T);
  catchOpenMCError(err, "get temperature of cell " + printCell(cell_info));
  return T;
}

void
OpenMCCellAverageProblem::reloadDAGMC()
{
#ifdef ENABLE_DAGMC
  _dagmc.reset(new moab::DagMC(_skinner->moabPtr(),
                               0.0 /* overlap tolerance, default */,
                               0.001 /* numerical precision, default */,
                               0 /* verbosity */));

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

  // Finalize cross sections; we manually change the verbosity here because if skinning is
  // enabled, we don't want to overwhelm the user with excess console output showing info
  // which ultimately is no different from that shown on initialization
  auto initial_verbosity = openmc::settings::verbosity;
  openmc::settings::verbosity = 1;
  openmc::finalize_cross_sections();

  // Needed to obtain correct cell instances
  openmc::prepare_distribcell();
  openmc::settings::verbosity = initial_verbosity;

  _console << "done" << std::endl;
#endif
}

void
OpenMCCellAverageProblem::addTallyObject(const std::string & type,
                                         const std::string & name,
                                         InputParameters & moose_object_pars)
{
  auto tally = addObject<TallyBase>(type, name, moose_object_pars, false)[0];
  _local_tallies.push_back(tally);
  _local_tally_score_map.emplace_back();

  const auto & tally_scores = tally->getScores();
  for (unsigned int i = 0; i < tally_scores.size(); ++i)
  {
    _local_tally_score_map.back()[tally_scores[i]] = i;

    // Add the local tally's score to the list of scores if we don't have it yet.
    if (std::find(_all_tally_scores.begin(), _all_tally_scores.end(), tally_scores[i]) ==
        _all_tally_scores.end())
      _all_tally_scores.push_back(tally_scores[i]);
  }

  _contains_cell_tally = type == "CellTally" ? true : _contains_cell_tally;

  // Add the associated global tally if required.
  if (_needs_global_tally && tally->getAuxVarNames().size() > 0)
  {
    _global_tally_scores.push_back(tally_scores);
    _global_tally_estimators.push_back(tally->getTallyEstimator());
  }
}

void
OpenMCCellAverageProblem::validateLocalTallies()
{
  // We can skip this check if we don't have tallies.
  if (_local_tallies.size() == 0)
    return;

  /**
   * Check to make sure local tallies don't share scores (unless they're distributed mesh tallies).
   * This prevents normalization issues as we sum the values of all of the scores over all of the
   * tally bins.
   * TODO: we might be able to loosen this restriction later if there's a good way to
   * account for bin overlap.
   */
  std::vector<unsigned int> tallies_per_score;
  tallies_per_score.resize(_all_tally_scores.size(), 0);
  for (unsigned int i = 0; i < _local_tallies.size(); ++i)
  {
    for (unsigned int global_score = 0; global_score < _all_tally_scores.size(); ++global_score)
    {
      bool has_score = _local_tally_score_map[i].count(_all_tally_scores[global_score]) == 1;
      // The second check is required to avoid multi counting translated mesh tallies.
      if (has_score && _local_tallies[i]->getAuxVarNames().size() > 0)
        tallies_per_score[global_score]++;
    }
  }

  for (unsigned int global_score = 0; global_score < _all_tally_scores.size(); ++global_score)
  {
    if (tallies_per_score[global_score] > 1)
    {
      mooseError("You have added " + Moose::stringify(tallies_per_score[global_score]) +
                 " tallies which score " + _all_tally_scores[global_score] +
                 "!\nCardinal does not support multiple tallies with the same"
                 " scores as these tallies may have overlapping bins, preventing normalization.");
    }
  }

  // need some special treatment for non-heating scores, in eigenvalue mode
  bool has_non_heating_score = false;
  for (const auto & t : _all_tally_scores)
    if (!isHeatingScore(t))
      has_non_heating_score = true;

  if (has_non_heating_score && _run_mode == openmc::RunMode::EIGENVALUE)
  {
    std::string non_heating_scores;
    for (const auto & e : _all_tally_scores)
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

    checkRequiredParam(_pars,
                       "source_rate_normalization",
                       "using a non-heating tally (" + non_heating_scores + ") in eigenvalue mode");
    const auto & norm = getParam<MooseEnum>("source_rate_normalization");

    // If the score is already in tally_score, no need to do anything special.
    std::string n = enumToTallyScore(norm);
    auto it = std::find(_all_tally_scores.begin(), _all_tally_scores.end(), n);
    if (it != _all_tally_scores.end())
      _source_rate_index = it - _all_tally_scores.begin();
    else if (it == _all_tally_scores.end() && _local_tallies.size() == 1)
    {
      if (_local_tallies[0]->renamesTallyVars())
        mooseError("When specifying 'name', the score indicated in "
                   "'source_rate_normalization' must be\n"
                   "listed in 'score' so that we know what you want to name that score (",
                   norm,
                   ")");

      // We can add the requested normalization score if and only if a single tally was added by
      // [Tallies].
      _all_tally_scores.push_back(n);
      _local_tallies[0]->addScore(n);
      _local_tally_score_map[0][n] = _local_tallies[0]->getScores().size() - 1;
      _global_tally_scores[0].push_back(n);
      _source_rate_index = _all_tally_scores.size() - 1;
    }
    else
    {
      // Otherwise, we error and let the user know that they need to add the score.
      mooseError("The local tallies added in the [Tallies] block do not contain the requested "
                 "heating score " +
                 n +
                 ". You must either add this score in one of the tallies or choose a different "
                 "heating score.");
    }
  }
  else if (isParamValid("source_rate_normalization"))
    mooseWarning(
        "When either running in fixed-source mode, or all tallies have units of eV/src, the "
        "'source_rate_normalization' parameter is unused!");
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

bool
OpenMCCellAverageProblem::cellMapsToSubdomain(const cellInfo & cell_info,
                                              const std::unordered_set<SubdomainID> & id) const
{
  auto s = _cell_to_elem_subdomain.at(cell_info);
  for (const auto & i : id)
    if (s.find(i) != s.end())
      return true;

  return false;
}

bool
OpenMCCellAverageProblem::cellHasIdenticalFill(const cellInfo & cell_info) const
{
  // material cells are discounted as identical fill
  const auto & cell = openmc::model::cells[cell_info.first];
  if (!_has_identical_cell_fills || cell->type_ == openmc::Fill::MATERIAL)
    return false;

  return cellMapsToSubdomain(cell_info, _identical_cell_fill_blocks);
}

OpenMCCellAverageProblem::containedCells
OpenMCCellAverageProblem::shiftCellInstances(const cellInfo & cell_info) const
{
  if (!_has_identical_cell_fills)
    mooseError("Internal error: should not call shiftCellInstances!");

  auto offset = _n_offset.at(cell_info);

  containedCells contained_cells;
  const auto & first_cell_cc = _cell_to_contained_material_cells.at(_first_identical_cell);
  for (const auto & cc : first_cell_cc)
  {
    const auto & index = cc.first;
    const auto & instances = cc.second;
    auto n_instances = instances.size();
    const auto & shifts = _instance_offsets.at(index);

    std::vector<int32_t> shifted_instances;
    for (unsigned int inst = 0; inst < n_instances; ++inst)
      shifted_instances.push_back(instances[inst] + offset * shifts[inst]);

    contained_cells[index] = shifted_instances;
  }

  return contained_cells;
}

OpenMCCellAverageProblem::containedCells
OpenMCCellAverageProblem::containedMaterialCells(const cellInfo & cell_info) const
{
  if (!cellHasIdenticalFill(cell_info))
    return _cell_to_contained_material_cells.at(cell_info);
  else
    return shiftCellInstances(cell_info);
}

std::vector<int32_t>
OpenMCCellAverageProblem::materialsInCells(const containedCells & contained_cells) const
{
  std::vector<int32_t> mats;
  for (const auto & contained : contained_cells)
  {
    for (const auto & instance : contained.second)
    {
      // we know this is a material cell, so we don't need to check that the fill is material
      int32_t material_index;
      cellInfo cell_info = {contained.first, instance};
      materialFill(cell_info, material_index);
      mats.push_back(material_index);
    }
  }

  return mats;
}

Point
OpenMCCellAverageProblem::transformPointToOpenMC(const Point & pt) const
{
  Point pnt_out = transformPoint(pt);

  // scale point to OpenMC domain
  pnt_out *= _scaling;

  return pnt_out;
}
#endif
