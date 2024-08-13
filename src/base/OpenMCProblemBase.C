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

#include "OpenMCProblemBase.h"
#include "CardinalAppTypes.h"
#include "AddTallyAction.h"

InputParameters
OpenMCProblemBase::validParams()
{
  InputParameters params = CardinalProblem::validParams();
  params.addParam<PostprocessorName>(
      "power", "Power (Watts) to normalize the OpenMC tallies; only used for k-eigenvalue mode");
  params.addParam<PostprocessorName>(
      "source_strength", "Neutrons/second to normalize the OpenMC tallies; only used for fixed source mode");
  params.addParam<bool>("verbose", false, "Whether to print diagnostic information");

  params.addParam<MooseEnum>("tally_type", getTallyTypeEnum(), "Type of tally to use in OpenMC");

  params.addRangeCheckedParam<Real>(
      "scaling",
      1.0,
      "scaling > 0.0",
      "Scaling factor to apply to [Mesh] to get to units of centimeters that OpenMC expects; "
      "setting 'scaling = 100.0', for instance, indicates that the [Mesh] is in units of meters");

  // interfaces to directly set some OpenMC parameters
  params.addRangeCheckedParam<unsigned int>(
      "openmc_verbosity",
      "openmc_verbosity >= 1 & openmc_verbosity <= 10",
      "OpenMC verbosity level; this overrides the setting in the XML files. Note that we cannot "
      "influence the verbosity of OpenMC's initialization routines, since these are run before "
      "Cardinal is initialized.");
  params.addRangeCheckedParam<unsigned int>(
      "inactive_batches",
      "inactive_batches >= 0",
      "Number of inactive batches to run in OpenMC; this overrides the setting in the XML files.");
  params.addRangeCheckedParam<int>("particles",
                                   "particles > 0 ",
                                   "Number of particles to run in each OpenMC batch; this "
                                   "overrides the setting in the XML files.");
  params.addRangeCheckedParam<unsigned int>(
      "batches",
      "batches > 0",
      "Number of batches to run in OpenMC; this overrides the setting in the XML files.");

  params.addParam<bool>("reuse_source",
                        false,
                        "Whether to take the initial fission source "
                        "for interation n to be the converged source bank from iteration n-1");
  params.addParam<bool>(
      "skip_statepoint",
      false,
      "Whether to skip writing any statepoint files from OpenMC; this is a performance "
      "optimization for scenarios where you may not want the statepoint files anyways");
  return params;
}

OpenMCProblemBase::OpenMCProblemBase(const InputParameters & params)
  : CardinalProblem(params),
    PostprocessorInterface(this),
    _verbose(getParam<bool>("verbose")),
    _reuse_source(getParam<bool>("reuse_source")),
    _specified_scaling(params.isParamSetByUser("scaling")),
    _scaling(getParam<Real>("scaling")),
    _skip_statepoint(getParam<bool>("skip_statepoint")),
    _fixed_point_iteration(-1),
    _total_n_particles(0)
{
  if (isParamValid("tally_type"))
    mooseError("The tally system used by OpenMCProblemBase derived classes has been deprecated. "
               "Please add tallies with the [Tallies] block instead.");

  int argc = 1;
  char openmc[] = "openmc";
  char * argv[1] = {openmc};

  openmc_init(argc, argv, &_communicator.get());

  // ensure that any mapped cells have their distribcell indices generated in OpenMC
  if (!openmc::settings::material_cell_offsets)
  {
    mooseWarning("Distributed properties for material cells are disabled "
                 "in the OpenMC settings. Enabling...");
    openmc::settings::material_cell_offsets = true;
    openmc::prepare_distribcell();
  }

  // ensure that unsupported run modes are not used, while also checking for
  // necessary/unused input parameters for the valid run modes
  _run_mode = openmc::settings::run_mode;
  const auto & tally_actions = getMooseApp().actionWarehouse().getActions<AddTallyAction>();
  switch (_run_mode)
  {
    case openmc::RunMode::EIGENVALUE:
    {
      // Jumping through hoops to see if we're going to add tallies down the line.
      if (tally_actions.size() > 0)
      {
        checkRequiredParam(params, "power", "running in k-eigenvalue mode");
        _power = &getPostprocessorValue("power");
      }
      else
        checkUnusedParam(params, "power", "no tallies have been added");

      checkUnusedParam(params, "source_strength", "running in k-eigenvalue mode");
      break;
    }
    case openmc::RunMode::FIXED_SOURCE:
    {
      if (tally_actions.size() > 0)
      {
        checkRequiredParam(params, "source_strength", "running in fixed source mode");
        _source_strength = &getPostprocessorValue("source_strength");
      }
      else
        checkUnusedParam(params, "source_strength", "no tallies have been added");

      checkUnusedParam(params, "inactive_batches", "running in fixed source mode");
      checkUnusedParam(params, "reuse_source", "running in fixed source mode");
      checkUnusedParam(params, "power", "running in fixed source mode");
      _reuse_source = false;
      break;
    }
    case openmc::RunMode::PLOTTING:
    case openmc::RunMode::PARTICLE:
    case openmc::RunMode::VOLUME:
      mooseError("Running OpenMC in plotting, particle, and volume modes is not supported through "
                 "Cardinal! Please just run using the OpenMC executable (e.g., openmc --plot for "
                 "plot mode).");
    default:
      mooseError("Unhandled openmc::RunMode enum in OpenMCInitAction!");
  }

  _n_cell_digits = std::to_string(openmc::model::cells.size()).length();

  if (openmc::settings::libmesh_comm)
    mooseWarning("libMesh communicator already set in OpenMC.");

  openmc::settings::libmesh_comm = &_mesh.comm();

  if (openmc::settings::temperature_range[1] == 0.0)
    mooseWarning(
        "For multiphysics simulations, we recommend setting the 'temperature_range' in OpenMC's "
        "settings.xml file. This will pre-load nuclear data over a range of temperatures, instead "
        "of only the temperatures defined in the XML file.\n\n"
        "For efficiency purposes, OpenMC only checks that cell temperatures are within the global "
        "min/max of loaded data, which can be different from data loaded for each nuclide. Run may "
        "abort suddenly if requested nuclear data is not available.");

  if (isParamValid("openmc_verbosity"))
    openmc::settings::verbosity = getParam<unsigned int>("openmc_verbosity");

  if (isParamValid("inactive_batches"))
    openmc::settings::n_inactive = getParam<unsigned int>("inactive_batches");

  if (isParamValid("particles"))
    openmc::settings::n_particles = getParam<int>("particles");

  if (isParamValid("batches"))
  {
    auto xml_n_batches = openmc::settings::n_batches;

    int err = openmc_set_n_batches(getParam<unsigned int>("batches"),
                                   true /* set the max batches */,
                                   true /* add the last batch for statepoint writing */);
    catchOpenMCError(err, "set the number of batches");

    // if we set the batches from Cardinal, remove whatever statepoint file was
    // created for the #batches set in the XML files; this is just to reduce the
    // number of statepoint files by removing an unnecessary point
    openmc::settings::statepoint_batch.erase(xml_n_batches);
  }

  // The OpenMC wrapping doesn't require material properties itself, but we might
  // define them on some blocks of the domain for other auxiliary kernel purposes
  setMaterialCoverageCheck(false);
}

OpenMCProblemBase::~OpenMCProblemBase() { openmc_finalize(); }

void
OpenMCProblemBase::catchOpenMCError(const int & err, const std::string descriptor) const
{
  if (err)
    mooseError("In attempting to ", descriptor, ", OpenMC reported:\n\n",
      std::string(openmc_err_msg));
}

void
OpenMCProblemBase::fillElementalAuxVariable(const unsigned int & var_num,
                                            const std::vector<unsigned int> & elem_ids,
                                            const Real & value)
{
  auto & solution = _aux->solution();
  auto sys_number = _aux->number();

  // loop over all the elements and set the specified variable to the specified value
  for (const auto & e : elem_ids)
  {
    auto elem_ptr = _mesh.queryElemPtr(e);

    if (!isLocalElem(elem_ptr))
      continue;

    auto dof_idx = elem_ptr->dof_number(sys_number, var_num, 0);
    solution.set(dof_idx, value);
  }
}

int
OpenMCProblemBase::nParticles() const
{
  return openmc::settings::n_particles;
}

std::string
OpenMCProblemBase::materialName(const int32_t index) const
{
  // OpenMC uses -1 to indicate void materials, which don't have a name. So we return
  // one ourselves, or else openmc_material_get_name will throw an error.
  if (index == -1)
    return "VOID";

  const char * name;
  int err = openmc_material_get_name(index, &name);
  catchOpenMCError(err, "get material name for material with index " + std::to_string(index));

  std::string n = name;

  // if the material does not have a name, just return the ID instead
  if (n.empty())
    n = std::to_string(materialID(index));

  return n;
}

int32_t
OpenMCProblemBase::cellID(const int32_t index) const
{
  int32_t id;
  int err = openmc_cell_get_id(index, &id);
  catchOpenMCError(err, "get ID for cell with index " + std::to_string(index));
  return id;
}

int32_t
OpenMCProblemBase::materialID(const int32_t index) const
{
  if (index == openmc::MATERIAL_VOID)
    return -1;

  int32_t id;
  int err = openmc_material_get_id(index, &id);
  catchOpenMCError(err, "get ID for material with index " + std::to_string(index));
  return id;
}

std::string
OpenMCProblemBase::printMaterial(const int32_t & index) const
{
  int32_t id = materialID(index);
  std::stringstream msg;
  msg << "material " << id;
  return msg.str();
}

std::string
OpenMCProblemBase::printPoint(const Point & p) const
{
  std::stringstream msg;
  msg << "(" << std::setprecision(6) << std::setw(7) << p(0) << ", " << std::setprecision(6)
      << std::setw(7) << p(1) << ", " << std::setprecision(6) << std::setw(7) << p(2) << ")";
  return msg.str();
}

bool
OpenMCProblemBase::firstSolve() const
{
  return _fixed_point_iteration < 0;
}

void
OpenMCProblemBase::externalSolve()
{
  TIME_SECTION("solveOpenMC", 1, "Solving OpenMC", false);
  _console << " Running OpenMC with " << nParticles() << " particles per batch..." << std::endl;

  // apply a new starting fission source
  if (_reuse_source && !firstSolve())
  {
    openmc::free_memory_source();
    openmc::model::external_sources.push_back(
        std::make_unique<openmc::FileSource>(sourceBankFileName()));
  }

  int err = openmc_run();
  if (err)
    mooseError(openmc_err_msg);

  _total_n_particles += nParticles();

  err = openmc_reset_timers();
  if (err)
    mooseError(openmc_err_msg);

  _fixed_point_iteration += 1;

  // save the latest fission source for re-use in the next iteration
  if (_reuse_source)
    writeSourceBank(sourceBankFileName());
}

void
OpenMCProblemBase::writeSourceBank(const std::string & filename)
{
  hid_t file_id = openmc::file_open(filename, 'w', true);
  openmc::write_attribute(file_id, "filetype", "source");
  openmc::write_source_bank(file_id, openmc::simulation::source_bank,
    openmc::simulation::work_index);
  openmc::file_close(file_id);
}

unsigned int
OpenMCProblemBase::numElemsInSubdomain(const SubdomainID & id) const
{
  unsigned int n = 0;
  for (unsigned int e = 0; e < _mesh.nElem(); ++e)
  {
    const auto * elem = _mesh.queryElemPtr(e);

    if (!isLocalElem(elem))
      continue;

    const auto subdomain_id = elem->subdomain_id();
    if (id == subdomain_id)
      n += 1;
  }

  _communicator.sum(n);

  return n;
}

bool
OpenMCProblemBase::isLocalElem(const Elem * elem) const
{
  if (!elem)
  {
    // we should only not be able to find an element if the mesh is distributed
    libmesh_assert(!_mesh.getMesh().is_serial());
    return false;
  }

  if (elem->processor_id() == _communicator.rank())
    return true;

  return false;
}

bool
OpenMCProblemBase::cellHasZeroInstances(const cellInfo & cell_info) const
{
  auto n = openmc::model::cells.at(cell_info.first)->n_instances_;
  return !n;
}

void
OpenMCProblemBase::setCellTemperature(const int32_t & index,
                                      const int32_t & instance,
                                      const Real & T,
                                      const cellInfo & cell_info) const
{
  int err = openmc_cell_set_temperature(index, T, &instance, false);
  if (err)
  {
    std::string descriptor =
        "set cell " + printCell(cell_info) + " to temperature " + Moose::stringify(T) + " (K)";

    // special error message if cell has zero instances
    if (cellHasZeroInstances(cell_info))
      mooseError("Failed to set the temperature for cell " + printCell(cell_info) +
                 " with zero instances.");

    mooseError("In attempting to set cell " + printCell(cell_info) + " to temperature " +
                   Moose::stringify(T) + " (K), OpenMC reported:\n\n",
               std::string(openmc_err_msg) + "\n\n" +
                   "If you are trying to debug a model setup, you can set 'initial_properties = "
                   "xml' to use the initial temperature and density in the OpenMC XML files for "
                   "OpenMC's first run.");
  }
}

std::vector<int32_t>
OpenMCProblemBase::cellFill(const cellInfo & cell_info, int & fill_type) const
{
  int32_t * materials = nullptr;
  int n_materials = 0;

  int err = openmc_cell_get_fill(cell_info.first, &fill_type, &materials, &n_materials);
  catchOpenMCError(err, "get fill of cell " + printCell(cell_info));

  std::vector<int32_t> material_indices;
  material_indices.assign(materials, materials + n_materials);
  return material_indices;
}

bool
OpenMCProblemBase::materialFill(const cellInfo & cell_info, int32_t & material_index) const
{
  int fill_type;
  auto material_indices = cellFill(cell_info, fill_type);

  if (fill_type != static_cast<int>(openmc::Fill::MATERIAL))
    return false;

  // The number of materials in a cell is either 1, or equal to the number of instances
  // (if distributed materials were used).
  if (material_indices.size() == 1)
    material_index = material_indices[0];
  else
    material_index = material_indices[cell_info.second];

  return true;
}

void
OpenMCProblemBase::setCellDensity(const Real & density, const cellInfo & cell_info) const
{
  // OpenMC technically allows a density of >= 0.0, but we can impose a tighter
  // check here with a better error message than the Excepts() in material->set_density
  // because it could be a very common mistake to forget to set an initial condition
  // for density if OpenMC runs first
  if (density <= 0.0)
    mooseError("Densities less than or equal to zero cannot be set in the OpenMC model!\n\n cell " +
               printCell(cell_info) + " set to density " + Moose::stringify(density) + " (kg/m3)");

  int32_t material_index;
  auto is_material_cell = materialFill(cell_info, material_index);

  if (!is_material_cell)
    mooseError("Density transfer does not currently support cells filled with universes or lattices!");

  // throw a special error if the cell is void, because the OpenMC error isn't very
  // clear what the mistake is
  if (material_index == MATERIAL_VOID)
    mooseError("Cannot set density for cell " + printCell(cell_info) +
               " because this cell is void (vacuum)!");

  // Multiply density by 0.001 to convert from kg/m3 (the units assumed in the 'density'
  // auxvariable as well as the MOOSE fluid properties module) to g/cm3
  const char * units = "g/cc";
  int err = openmc_material_set_density(
      material_index, density * _density_conversion_factor, units);

  if (err)
  {
    // special error message if cell has zero instances
    if (cellHasZeroInstances(cell_info))
      mooseError("Failed to set the density for cell " + printCell(cell_info) +
                 " with zero instances.");

    mooseError("In attempting to set cell " + printCell(cell_info) + " to density " +
                   Moose::stringify(density) + " (kg/m3), OpenMC reported:\n\n",
               std::string(openmc_err_msg) + "\n\n" +
                   "If you are trying to debug a model setup, you can set 'initial_properties = "
                   "xml' to use the initial temperature and density in the OpenMC XML files for "
                   "OpenMC's first run.");
  }
}

std::string
OpenMCProblemBase::printCell(const cellInfo & cell_info, const bool brief) const
{
  int32_t id = cellID(cell_info.first);

  std::stringstream msg;
  if (!brief)
    msg << "id ";

  msg << std::setw(_n_cell_digits) << Moose::stringify(id) << ", instance "
      << std::setw(_n_cell_digits) << Moose::stringify(cell_info.second) << " (of "
      << std::setw(_n_cell_digits)
      << Moose::stringify(openmc::model::cells.at(cell_info.first)->n_instances_) << ")";

  return msg.str();
}

void
OpenMCProblemBase::importProperties() const
{
  _console << "Reading temperature and density from properties.h5" << std::endl;

  int err = openmc_properties_import("properties.h5");
  catchOpenMCError(err, "load temperature and density from a properties.h5 file");
}

xt::xtensor<double, 1>
OpenMCProblemBase::relativeError(const xt::xtensor<double, 1> & sum,
                                 const xt::xtensor<double, 1> & sum_sq,
                                 const int & n_realizations) const
{
  xt::xtensor<double, 1> rel_err = xt::zeros<double>({sum.size()});

  for (unsigned int i = 0; i < sum.size(); ++i)
  {
    auto mean = sum(i) / n_realizations;
    auto std_dev = std::sqrt((sum_sq(i) / n_realizations - mean * mean) / (n_realizations - 1));
    rel_err[i] = mean != 0.0 ? std_dev / std::abs(mean) : 0.0;
  }

  return rel_err;
}

xt::xtensor<double, 1>
OpenMCProblemBase::tallySum(openmc::Tally * tally, const unsigned int & score) const
{
  return xt::view(tally->results_, xt::all(), score, static_cast<int>(openmc::TallyResult::SUM));
}

double
OpenMCProblemBase::tallySumAcrossBins(std::vector<openmc::Tally *> tally, const unsigned int & score) const
{
  double sum = 0.0;

  for (const auto & t : tally)
  {
    auto mean = tallySum(t, score);
    sum += xt::sum(mean)();
  }

  return sum;
}

double
OpenMCProblemBase::tallyMeanAcrossBins(std::vector<openmc::Tally *> tally, const unsigned int & score) const
{
  int n = 0;
  for (const auto & t : tally)
    n += t->n_realizations_;

  return tallySumAcrossBins(tally, score) / n;
}

std::string
OpenMCProblemBase::enumToTallyScore(const std::string & score) const
{
  // the MultiMooseEnum is all caps, but the MooseEnum is already the correct case,
  // so we need to treat these as separate
  std::string s = score;
  if (std::all_of(
          s.begin(), s.end(), [](unsigned char c) { return !std::isalpha(c) || std::isupper(c); }))
  {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return std::tolower(c); });

    // we need to revert back to some letters being uppercase for certain scores
    if (s == "h3_production")
      s = "H3_production";
  }

  // MOOSE enums use underscores, OpenMC uses dashes
  std::replace(s.begin(), s.end(), '_', '-');
  return s;
}

std::string
OpenMCProblemBase::tallyScoreToEnum(const std::string & score) const
{
  // MOOSE enums use underscores, OpenMC uses dashes
  std::string s = score;
  std::replace(s.begin(), s.end(), '-', '_');
  return s;
}

openmc::TallyEstimator
OpenMCProblemBase::tallyEstimator(tally::TallyEstimatorEnum estimator) const
{
  switch (estimator)
  {
    case tally::tracklength:
      return openmc::TallyEstimator::TRACKLENGTH;
    case tally::collision:
      return openmc::TallyEstimator::COLLISION;
    case tally::analog:
      return openmc::TallyEstimator::ANALOG;
    default:
      mooseError("Unhandled TallyEstimatorEnum!");
  }
}

std::string
OpenMCProblemBase::estimatorToString(openmc::TallyEstimator estimator) const
{
  switch (estimator)
  {
    case openmc::TallyEstimator::TRACKLENGTH:
      return "tracklength";
    case openmc::TallyEstimator::COLLISION:
      return "collision";
    case openmc::TallyEstimator::ANALOG:
      return "analog";
    default:
      mooseError("Unhandled TallyEstimatorEnum!");
  }
}

openmc::TriggerMetric
OpenMCProblemBase::triggerMetric(std::string trigger) const
{
  if (trigger == "variance")
    return openmc::TriggerMetric::variance;
  else if (trigger == "std_dev")
    return openmc::TriggerMetric::standard_deviation;
  else if (trigger == "rel_err")
    return openmc::TriggerMetric::relative_error;
  else if (trigger == "none")
    return openmc::TriggerMetric::not_active;
  else
    mooseError("Unhandled TallyTriggerTypeEnum: ", trigger);
}

openmc::TriggerMetric
OpenMCProblemBase::triggerMetric(trigger::TallyTriggerTypeEnum trigger) const
{
  switch (trigger)
  {
    case trigger::variance:
      return openmc::TriggerMetric::variance;
    case trigger::std_dev:
      return openmc::TriggerMetric::standard_deviation;
    case trigger::rel_err:
      return openmc::TriggerMetric::relative_error;
    case trigger::none:
      return openmc::TriggerMetric::not_active;
    default:
      mooseError("Unhandled TallyTriggerTypeEnum!");
  }
}

bool
OpenMCProblemBase::cellIsVoid(const cellInfo & cell_info) const
{
  // material_index will be unchanged if the cell is filled by a universe or lattice.
  // Otherwise, this will get set to the material index in the cell.
  int32_t material_index = 0;
  materialFill(cell_info, material_index);
  return material_index == MATERIAL_VOID;
}

void
OpenMCProblemBase::geometryType(bool & has_csg_universe, bool & has_dag_universe) const
{
  has_csg_universe = false;
  has_dag_universe = false;

  // Loop over universes and check if type is DAGMC
  for (const auto& universe: openmc::model::universes)
  {
    if (universe->geom_type() == openmc::GeometryType::DAG)
      has_dag_universe = true;
    else if (universe->geom_type() == openmc::GeometryType::CSG)
      has_csg_universe = true;
    else
      mooseError("Unhandled GeometryType enum!");
  }
}

long unsigned int
OpenMCProblemBase::numCells() const
{
  long unsigned int n_openmc_cells = 0;
  for (const auto & c : openmc::model::cells)
    n_openmc_cells += c->n_instances_;

  return n_openmc_cells;
}

bool
OpenMCProblemBase::isHeatingScore(const std::string & score) const
{
  std::set<std::string> viable_scores = {"heating", "heating-local", "kappa-fission",
    "fission-q-prompt", "fission-q-recoverable", "damage-energy"};
  return viable_scores.count(score);
}

unsigned int
OpenMCProblemBase::addExternalVariable(const std::string & name, const std::vector<SubdomainName> * block)
{
  auto var_params = _factory.getValidParams("MooseVariable");
  var_params.set<MooseEnum>("family") = "MONOMIAL";
  var_params.set<MooseEnum>("order") = "CONSTANT";

  if (block)
    var_params.set<std::vector<SubdomainName>>("block") = *block;

  checkDuplicateVariableName(name);
  addAuxVariable("MooseVariable", name, var_params);
  return _aux->getFieldVariable<Real>(0, name).number();
}

std::string
OpenMCProblemBase::subdomainName(const SubdomainID & id) const
{
  std::string name = _mesh.getSubdomainName(id);
  if (name.empty())
    name = std::to_string(id);
  return name;
}

void
OpenMCProblemBase::getOpenMCUserObjects()
{
  TheWarehouse::Query uo_query = theWarehouse().query().condition<AttribSystem>("UserObject");
  std::vector<UserObject *> userobjs;
  uo_query.queryInto(userobjs);

  for (const auto & u : userobjs)
  {
    OpenMCNuclideDensities * c = dynamic_cast<OpenMCNuclideDensities *>(u);
    if (c)
      _nuclide_densities_uos.push_back(c);

    OpenMCTallyNuclides * d = dynamic_cast<OpenMCTallyNuclides *>(u);
    if (d)
      _tally_nuclides_uos.push_back(d);
  }
}

void
OpenMCProblemBase::sendNuclideDensitiesToOpenMC()
{
  if (_nuclide_densities_uos.size() == 0)
    return;

  // We could probably put this somewhere better, but it's good for now
  executeControls(EXEC_SEND_OPENMC_DENSITIES);

  _console << "Sending nuclide compositions to OpenMC... ";
  for (const auto & uo : _nuclide_densities_uos)
    uo->setValue();
  _console << "done" << std::endl;
}

void
OpenMCProblemBase::sendTallyNuclidesToOpenMC()
{
  if (_tally_nuclides_uos.size() == 0)
    return;

  // We could probably put this somewhere better, but it's good for now
  executeControls(EXEC_SEND_OPENMC_TALLY_NUCLIDES);

  _console << "Sending tally nuclides to OpenMC... ";
  for (const auto & uo : _tally_nuclides_uos)
    uo->setValue();
  _console << "done" << std::endl;
}

#endif
