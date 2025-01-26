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

#include "OpenMCAuxKernel.h"

template <typename ComputeValueType>
InputParameters
OpenMCAuxKernelTempl<ComputeValueType>::validParams()
{
  InputParameters params = AuxKernelTempl<ComputeValueType>::validParams();
  params += OpenMCBase::validParams();
  return params;
}

template <typename ComputeValueType>
OpenMCAuxKernelTempl<ComputeValueType>::OpenMCAuxKernelTempl(const InputParameters & parameters)
  : AuxKernelTempl<ComputeValueType>(parameters), OpenMCBase(this, parameters)
{
  if (this->isNodal())
    mooseError("This auxkernel can only be used with elemental variables!");
}

template <typename ComputeValueType>
bool
OpenMCAuxKernelTempl<ComputeValueType>::mappedElement()
{
  OpenMCCellAverageProblem::cellInfo cell_info =
      _openmc_problem->elemToCellInfo(this->_current_elem->id());
  return !(cell_info.first == OpenMCCellAverageProblem::UNMAPPED);
}

template <typename ComputeValueType>
std::vector<const MooseVariableFE<Real> *>
OpenMCAuxKernelTempl<ComputeValueType>::getTallyScoreVariables(const std::string & score)
{
  std::vector<const MooseVariableFE<Real> *> score_vars;
  const auto & tallies = _openmc_problem->getLocalTally();
  for (const auto & t : tallies)
  {
    if (t->hasScore(score))
    {
      auto vars = t->getScoreVars(score);
      for (const auto & v : vars)
        score_vars.emplace_back(dynamic_cast<const MooseVariableFE<Real> *>(
            &this->_subproblem.getVariable(this->_tid, v)));
    }
  }

  if (score_vars.size() == 0)
    mooseError("No tallies contain the requested score " + score + "!");

  return score_vars;
}

template <typename ComputeValueType>
std::vector<const VariableValue *>
OpenMCAuxKernelTempl<ComputeValueType>::getTallyScoreVariableValues(const std::string & score)
{
  std::vector<const VariableValue *> score_vars;
  const auto & tallies = _openmc_problem->getLocalTally();
  for (const auto & t : tallies)
  {
    if (t->hasScore(score))
    {
      auto vars = t->getScoreVars(score);
      for (const auto & v : vars)
        score_vars.emplace_back(
            &(dynamic_cast<MooseVariableFE<Real> *>(&this->_subproblem.getVariable(this->_tid, v))->sln()));
    }
  }

  if (score_vars.size() == 0)
    mooseError("No tallies contain the requested score " + score + "!");

  return score_vars;
}

template <typename ComputeValueType>
std::vector<const VariableValue *>
OpenMCAuxKernelTempl<ComputeValueType>::getTallyScoreNeighborVariableValues(const std::string & score)
{
  std::vector<const VariableValue *> score_vars;
  const auto & tallies = _openmc_problem->getLocalTally();
  for (const auto & t : tallies)
  {
    if (t->hasScore(score))
    {
      auto vars = t->getScoreVars(score);
      for (const auto & v : vars)
        score_vars.emplace_back(
            &(dynamic_cast<MooseVariableFE<Real> *>(&this->_subproblem.getVariable(this->_tid, v))->slnNeighbor()));
    }
  }

  if (score_vars.size() == 0)
    mooseError("No tallies contain the requested score " + score + "!");

  return score_vars;
}

// Explicitly instantiates the three versions of the OpenMCAuxKernelTempl class
template class OpenMCAuxKernelTempl<Real>;
template class OpenMCAuxKernelTempl<RealVectorValue>;
template class OpenMCAuxKernelTempl<RealEigenVector>;

#endif
