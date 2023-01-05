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
#include "NonlinearSystem.h"
#include "AuxiliarySystem.h"
#include "UserErrorChecking.h"

#include "mpi.h"
#include "openmc/capi.h"
#include "openmc/cell.h"
#include "openmc/geometry.h"
#include "openmc/hdf5_interface.h"
#include "openmc/material.h"
#include "openmc/mesh.h"
#include "openmc/settings.h"
#include "openmc/source.h"
#include "openmc/state_point.h"
#include "xtensor/xview.hpp"

InputParameters
OpenMCProblemBase::validParams()
{
  InputParameters params = ExternalProblem::validParams();
  params.addParam<PostprocessorName>(
      "power", "Power (Watts) to normalize the OpenMC tallies; only used for k-eigenvalue mode");
  params.addParam<PostprocessorName>(
      "source_strength", "Neutrons/second to normalize the OpenMC tallies; only used for fixed source mode");
  params.addParam<bool>("verbose", false, "Whether to print diagnostic information");

  // interfaces to directly set some OpenMC parameters
  params.addRangeCheckedParam<unsigned int>(
      "openmc_verbosity",
      "openmc_verbosity >= 1 & openmc_verbosity <= 10",
      "OpenMC verbosity level; this overrides the setting in the XML files");
  params.addRangeCheckedParam<unsigned int>(
      "inactive_batches",
      "inactive_batches > 0",
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
  return params;
}

OpenMCProblemBase::OpenMCProblemBase(const InputParameters & params)
  : ExternalProblem(params),
    PostprocessorInterface(this),
    _verbose(getParam<bool>("verbose")),
    _reuse_source(getParam<bool>("reuse_source")),
    _single_coord_level(openmc::model::n_coord_levels == 1),
    _fixed_point_iteration(-1),
    _path_output(openmc::settings::path_output),
    _n_cell_digits(std::to_string(openmc::model::cells.size()).length())
{
  if (openmc::settings::run_mode == openmc::RunMode::FIXED_SOURCE)
  {
    checkRequiredParam(params, "source_strength", "running in fixed source mode");
    _source_strength = &getPostprocessorValue("source_strength");

    checkUnusedParam(params, "inactive_batches", "running in fixed source mode");
    checkUnusedParam(params, "reuse_source", "running in fixed source mode");
    checkUnusedParam(params, "power", "running in fixed source mode");
    _reuse_source = false;
  }
  else
  {
    checkRequiredParam(params, "power", "running in k-eigenvalue mode");
    _power = &getPostprocessorValue("power");

    checkUnusedParam(params, "source_strength", "running in k-eigenvalue mode");
  }

  if (openmc::settings::libmesh_comm)
    mooseWarning("libMesh communicator already set in OpenMC.");

  openmc::settings::libmesh_comm = &_mesh.comm();

  if (openmc::settings::temperature_range[1] == 0.0)
    mooseWarning(
      "For multiphysics simulations, we recommend setting the 'temperature_range' in OpenMC's\n"
      "settings.xml file. This will pre-load nuclear data over a range of temperatures, instead\n"
      "of only the temperatures defined in the XML file.\n\n"
      "For efficiency purposes, OpenMC only checks that cell temperatures are within the global\n"
      "min/max of loaded data, which can be different from data loaded for each nuclide. Run may\n"
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

    if (err)
      mooseError("In attempting to set the number of batches, OpenMC reported:\n\n" +
                 std::string(openmc_err_msg));

    // if we set the batches from Cardinal, remove whatever statepoint file was
    // created for the #batches set in the XML files; this is just to reduce the
    // number of statepoint files by removing an unnecessary point
    openmc::settings::statepoint_batch.erase(xml_n_batches);
  }

  // The OpenMC wrapping doesn't require material properties itself, but we might
  // define them on some blocks of the domain for other auxiliary kernel purposes
  setMaterialCoverageCheck(false);

  _n_openmc_cells = 0.0;
  for (const auto & c : openmc::model::cells)
    _n_openmc_cells += c->n_instances_;

  // establish the local -> global element mapping for convenience
  for (unsigned int e = 0; e < _mesh.nElem(); ++e)
  {
    const auto * elem = _mesh.queryElemPtr(e);
    if (!isLocalElem(elem))
      continue;

    _local_to_global_elem.push_back(e);
  }
}

OpenMCProblemBase::~OpenMCProblemBase() { openmc_finalize(); }

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

int32_t
OpenMCProblemBase::cellID(const int32_t index) const
{
  int32_t id;
  int err = openmc_cell_get_id(index, &id);

  if (err)
    mooseError("In attempting to get ID for cell with index " + Moose::stringify(index) +
               " , OpenMC reported:\n\n" + std::string(openmc_err_msg));

  return id;
}

int32_t
OpenMCProblemBase::materialID(const int32_t index) const
{
  int32_t id;
  int err = openmc_material_get_id(index, &id);

  if (err)
  {
    std::stringstream msg;
    msg << "In attempting to get ID for material with index " + Moose::stringify(index) +
               ", OpenMC reported:\n\n" + std::string(openmc_err_msg);
  }

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

void
OpenMCProblemBase::externalSolve()
{
  TIME_SECTION("solveOpenMC", 1, "Solving OpenMC", false);
  _console << " Running OpenMC with " << nParticles() << " particles per batch..." << std::endl;

  bool first_iteration = _fixed_point_iteration < 0;

  // apply a new starting fission source
  if (_reuse_source && !first_iteration)
  {
    openmc::free_memory_source();
    openmc::model::external_sources.push_back(
        std::make_unique<openmc::FileSource>(sourceBankFileName()));
  }

  int err = openmc_run();
  if (err)
    mooseError(openmc_err_msg);

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
  openmc::write_source_bank(file_id, false);
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
    libmesh_assert(!_mesh.is_serial());
    return false;
  }

  if (elem->processor_id() == _communicator.rank())
    return true;

  return false;
}

void
OpenMCProblemBase::setCellTemperature(const int32_t & id, const int32_t & instance, const Real & T,
  const cellInfo & cell_info) const
{
  int err = openmc_cell_set_temperature(id, T, &instance, false);

  if (err)
    mooseError("In attempting to set cell " + printCell(cell_info) + " to temperature " +
               Moose::stringify(T) + " (K), OpenMC reported:\n\n" +
               std::string(openmc_err_msg));
}

std::vector<int32_t>
OpenMCProblemBase::cellFill(const cellInfo & cell_info, int & fill_type) const
{
  fill_type = static_cast<int>(openmc::Fill::MATERIAL);
  int32_t * materials = nullptr;
  int n_materials = 0;

  int err = openmc_cell_get_fill(cell_info.first, &fill_type, &materials, &n_materials);

  if (err)
    mooseError("In attempting to get fill of cell " + printCell(cell_info) +
               ", OpenMC reported:\n\n" + std::string(openmc_err_msg));

  std::vector<int32_t> material_indices;
  material_indices.assign(materials, materials + n_materials);
  return material_indices;
}

void
OpenMCProblemBase::setCellDensity(const Real & density, const cellInfo & cell_info) const
{
  // OpenMC technically allows a density of >= 0.0, but we can impose a tighter
  // check here with a better error message than the Excepts() in material->set_density
  // because it could be a very common mistake to forget to set an initial condition
  // for density if OpenMC runs first
  if (density <= 0.0)
    mooseError("Densities less than or equal to zero cannot be set in the OpenMC model!\n cell " +
               printCell(cell_info) + " set to density " + Moose::stringify(density) + " (kg/m3)");

  int fill_type;
  std::vector<int32_t> material_indices = cellFill(cell_info, fill_type);

  // throw a special error if the cell is void, because the OpenMC error isn't very
  // clear what the mistake is
  if (material_indices[0] == MATERIAL_VOID)
    mooseError("Cannot set density for cell " + printCell(cell_info) +
               " because this cell is void (vacuum)!");

  if (fill_type != static_cast<int>(openmc::Fill::MATERIAL))
    mooseError(
        "Density transfer does not currently support cells filled with universes or lattices!");

  // Multiply density by 0.001 to convert from kg/m3 (the units assumed in the 'density'
  // auxvariable as well as the MOOSE fluid properties module) to g/cm3
  const char * units = "g/cc";
  int err = openmc_material_set_density(
      material_indices[cell_info.second], density * _density_conversion_factor, units);

  if (err)
    mooseError("In attempting to set material with index " +
               Moose::stringify(material_indices[cell_info.second]) + " to density " +
               Moose::stringify(density) + " (kg/m3), OpenMC reported:\n\n" +
               std::string(openmc_err_msg));
}

std::string
OpenMCProblemBase::printCell(const cellInfo & cell_info) const
{
  int32_t id = cellID(cell_info.first);

  std::stringstream msg;
  msg << "id " << std::setw(_n_cell_digits) << Moose::stringify(id) << ", instance "
      << std::setw(_n_cell_digits) << Moose::stringify(cell_info.second) << " (of "
      << std::setw(_n_cell_digits)
      << Moose::stringify(openmc::model::cells[cell_info.first]->n_instances_) << ")";

  return msg.str();
}

void
OpenMCProblemBase::importProperties() const
{
  _console << "Reading temperature and density from properties.h5" << std::endl;

  int err = openmc_properties_import("properties.h5");
  if (err)
    mooseError("In attempting to load temperature and density from a properties.h5 file, "
               "OpenMC reported:\n\n" +
               std::string(openmc_err_msg));
}

bool
OpenMCProblemBase::cellHasFissileMaterials(const cellInfo & cell_info) const
{
  int fill_type;
  std::vector<int32_t> material_indices = cellFill(cell_info, fill_type);

  // TODO: for cells with non-material fills, we need to implement something that recurses
  // into the cell/universe fills to see if there's anything fissile; until then, just assume
  // that the cell has something fissile
  if (fill_type != static_cast<int>(openmc::Fill::MATERIAL))
    return true;

  // for each material fill, check whether it is fissionable
  for (const auto & index : material_indices)
  {
    // We know void cells certainly aren't fissionable; if not void, check if fissionable
    if (index != MATERIAL_VOID)
    {
      const auto & material = openmc::model::materials[index];
      if (material->fissionable_)
        return true;
    }
  }

  return false;
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

void
OpenMCProblemBase::checkDuplicateVariableName(const std::string & name) const
{
  if (_aux.get()->hasVariable(name))
    mooseError("Cardinal is trying to add an auxiliary variable named '", name,
      "', but you already have a variable by this name. Please choose a different name "
      "for the auxiliary variable you are adding.");

  if (_nl[0].get()->hasVariable(name))
    mooseError("Cardinal is trying to add a nonlinear variable named '", name,
      "', but you already have a variable by this name. Please choose a different name "
      "for the nonlinear variable you are adding.");
}

#endif
