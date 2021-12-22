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

#include "OpenMCProblemBase.h"
#include "AuxiliarySystem.h"

#include "mpi.h"
#include "openmc/capi.h"
#include "openmc/cell.h"
#include "openmc/constants.h"
#include "openmc/error.h"
#include "openmc/material.h"
#include "openmc/particle.h"
#include "openmc/geometry.h"
#include "openmc/geometry_aux.h"
#include "openmc/message_passing.h"
#include "openmc/random_lcg.h"
#include "openmc/settings.h"
#include "openmc/summary.h"
#include "xtensor/xarray.hpp"
#include "xtensor/xview.hpp"

InputParameters
OpenMCProblemBase::validParams()
{
  InputParameters params = ExternalProblem::validParams();
  params.addRequiredRangeCheckedParam<Real>("power", "power >= 0.0",
    "Power (Watts) to normalize the OpenMC tallies");
  params.addParam<bool>("verbose", false, "Whether to print diagnostic information");
  params.addRangeCheckedParam<unsigned int>("inactive_batches", "inactive_batches > 0",
    "Number of inactive batches to run in OpenMC; this overrides the setting in the XML files.");
  params.addRangeCheckedParam<unsigned int>("batches", "batches > 0",
    "Number of batches to run in OpenMC; this overrides the setting in the XML files.");
  params.addRangeCheckedParam<unsigned int>("openmc_verbosity", "openmc_verbosity >= 1 & openmc_verbosity <= 10",
    "OpenMC verbosity level");
  return params;
}

OpenMCProblemBase::OpenMCProblemBase(const InputParameters &params) :
  ExternalProblem(params),
  _serialized_solution(NumericVector<Number>::build(_communicator).release()),
  _mesh_base(_mesh.getMesh()),
  _power(getParam<Real>("power")),
  _verbose(getParam<bool>("verbose")),
  _single_coord_level(openmc::model::n_coord_levels == 1),
  _n_cell_digits(digits(openmc::model::cells.size()))
{
  if (openmc::settings::libmesh_comm)
    mooseWarning("libMesh communicator already set in OpenMC.");

  openmc::settings::libmesh_comm = &_mesh.comm();

  if (isParamValid("inactive_batches"))
    openmc::settings::n_inactive = getParam<unsigned int>("inactive_batches");

  if (isParamValid("batches"))
  {
    int err = openmc_set_n_batches(getParam<unsigned int>("batches"),
      true /* set the max batches if triggers are used */,
      true /* add the last batch for statepoint writing */);

    if (err)
      mooseError("In attempting to set the number of batches, OpenMC reported:\n\n" +
        std::string(openmc_err_msg));
  }

  if (isParamValid("openmc_verbosity"))
    openmc::settings::verbosity = getParam<unsigned int>("openmc_verbosity");

  // for cases where OpenMC is the master app and we have two sub-apps that represent (1) fluid region,
  // and (2) solid region, we can save on one transfer if OpenMC computes the heat flux from a transferred
  // temperature (as opposed to the solid app sending both temperature and heat flux). Temperature is always
  // transferred. Because we need a material property to represent thermal conductivity, MOOSE's default
  // settings will force OpenMC to have materials on every block, when that's not actually needed. So
  // we can turn that check off.
  setMaterialCoverageCheck(false);

  _n_openmc_cells = 0.0;
  for (const auto & c : openmc::model::cells)
    _n_openmc_cells += c->n_instances_;
}

OpenMCProblemBase::~OpenMCProblemBase()
{
  openmc_finalize();
}

int
OpenMCProblemBase::digits(const int & number) const
{
  return std::to_string(number).length();
}

void
OpenMCProblemBase::setParticles(const int64_t & n) const
{
  openmc::settings::n_particles = n;
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
OpenMCProblemBase::printPoint(const Point & p) const
{
  std::stringstream msg;
  msg << "(" << std::setprecision(6) << std::setw(7) << p(0) << ", " <<
                std::setprecision(6) << std::setw(7) << p(1) << ", " <<
                std::setprecision(6) << std::setw(7) << p(2) << ")";
  return msg.str();
}

std::string
OpenMCProblemBase::printMaterial(const int32_t & index) const
{
  int32_t id = materialID(index);
  std::stringstream msg;
  msg << "material " << id;
  return msg.str();
}

double
OpenMCProblemBase::tallySum(std::vector<openmc::Tally *> tally) const
{
  double sum = 0.0;

  for (const auto & t : tally)
  {
    auto mean = xt::view(t->results_, xt::all(), 0, static_cast<int>(openmc::TallyResult::SUM));
    sum += xt::sum(mean)();
  }

  return sum;
}

Real
OpenMCProblemBase::relativeError(const Real & sum, const Real & sum_sq, const int & n_realizations) const
{
  Real mean = sum / n_realizations;
  Real std_dev = std::sqrt((sum_sq / n_realizations - mean * mean) / (n_realizations - 1));
  return mean != 0.0 ? std_dev / std::abs(mean) : 0.0;
}

void
OpenMCProblemBase::fillElementalAuxVariable(const unsigned int & var_num,
  const std::vector<unsigned int> & elem_ids, const Real & value)
{
  auto & solution = _aux->solution();
  auto sys_number = _aux->number();

  // loop over all the elements and set the specified variable to the specified value
  for (const auto & e : elem_ids)
  {
    auto elem_ptr = _mesh_base.query_elem_ptr(e);
    if (elem_ptr)
    {
      auto dof_idx = elem_ptr->dof_number(sys_number, var_num, 0);
      solution.set(dof_idx, value);
    }
  }
}
