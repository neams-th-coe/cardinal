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
#include "AuxiliarySystem.h"

#include "mpi.h"
#include "openmc/capi.h"
#include "openmc/cell.h"
#include "openmc/geometry.h"
#include "openmc/hdf5_interface.h"
#include "openmc/mesh.h"
#include "openmc/settings.h"
#include "openmc/source.h"
#include "openmc/state_point.h"

InputParameters
OpenMCProblemBase::validParams()
{
  InputParameters params = ExternalProblem::validParams();
  params.addRequiredRangeCheckedParam<Real>(
      "power", "power >= 0.0", "Power (Watts) to normalize the OpenMC tallies");
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
  params.addRangeCheckedParam<int64_t>("particles",
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
    _power(getParam<Real>("power")),
    _verbose(getParam<bool>("verbose")),
    _reuse_source(getParam<bool>("reuse_source")),
    _single_coord_level(openmc::model::n_coord_levels == 1),
    _fixed_point_iteration(-1),
    _path_output(openmc::settings::path_output)
{
  if (openmc::settings::libmesh_comm)
    mooseWarning("libMesh communicator already set in OpenMC.");

  openmc::settings::libmesh_comm = &_mesh.comm();

  if (isParamValid("openmc_verbosity"))
    openmc::settings::verbosity = getParam<unsigned int>("openmc_verbosity");

  if (isParamValid("inactive_batches"))
    openmc::settings::n_inactive = getParam<unsigned int>("inactive_batches");

  if (isParamValid("particles"))
    openmc::settings::n_particles = getParam<int64_t>("particles");

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

void
OpenMCProblemBase::fillElementalAuxVariable(const unsigned int & var_num,
                                            const std::vector<unsigned int> & elem_ids,
                                            const Real & value)
{
  auto & solution = _aux->solution();
  auto sys_number = _aux->number();
  const auto & mesh = _mesh.getMesh();

  // loop over all the elements and set the specified variable to the specified value
  for (const auto & e : elem_ids)
  {
    auto elem_ptr = mesh.query_elem_ptr(e);
    if (elem_ptr)
    {
      auto dof_idx = elem_ptr->dof_number(sys_number, var_num, 0);
      solution.set(dof_idx, value);
    }
  }
}

const int64_t &
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

#endif
