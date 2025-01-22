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

#include "OpenMCBase.h"

#include "libmesh/elem.h"

InputParameters
OpenMCBase::validParams()
{
  InputParameters params = emptyInputParameters();
  return params;
}

OpenMCBase::OpenMCBase(const MooseObject * moose_object, const InputParameters & parameters)
  : _openmc_problem(
        dynamic_cast<OpenMCCellAverageProblem *>(&moose_object->getMooseApp().feProblem()))
{
  if (!_openmc_problem)
    mooseError(moose_object->type() +
               " can only be used with problems of type 'OpenMCCellAverageProblem'!");

  // Check to make sure this object acts on a elemental variable (if it acts on a variable at all).
  if (parameters.isParamValid("variable"))
  {
    const auto var_name = parameters.getMooseType("variable");
    const auto sys = parameters.getCheckedPointerParam<SystemBase *>("_sys");
    const auto tid = parameters.get<THREAD_ID>("_tid");
    if (sys->getVariable(tid, var_name).isNodal())
      mooseError(moose_object->type() + " can only be used with elemental variables!");
  }
}

bool
OpenMCBase::mappedElement(const Elem * elem) const
{
  OpenMCCellAverageProblem::cellInfo cell_info = _openmc_problem->elemToCellInfo(elem->id());
  return !(cell_info.first == OpenMCCellAverageProblem::UNMAPPED);
}

std::vector<const MooseVariableFE<Real> *>
OpenMCBase::getTallyScoreVariables(const std::string & score, THREAD_ID t_id) const
{
  std::vector<const MooseVariableFE<Real> *> score_vars;
  const auto & tallies = _openmc_problem->getLocalTally();
  for (const auto & t : tallies)
  {
    if (t->hasScore(score))
    {
      auto vars = t->getScoreVars(score);
      for (const auto & v : vars)
        score_vars.emplace_back(
            dynamic_cast<const MooseVariableFE<Real> *>(&_openmc_problem->getVariable(t_id, v)));
    }
  }

  if (score_vars.size() == 0)
    mooseError("No tallies contain the requested score " + score + "!");

  return score_vars;
}

std::vector<const VariableValue *>
OpenMCBase::getTallyScoreVariableValues(const std::string & score, THREAD_ID t_id) const
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
            &(dynamic_cast<const MooseVariableFE<Real> *>(&_openmc_problem->getVariable(t_id, v))
                  ->sln()));
    }
  }

  if (score_vars.size() == 0)
    mooseError("No tallies contain the requested score " + score + "!");

  return score_vars;
}

std::vector<const VariableValue *>
OpenMCBase::getTallyScoreNeighborVariableValues(const std::string & score, THREAD_ID t_id) const
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
            &(dynamic_cast<const MooseVariableFE<Real> *>(&_openmc_problem->getVariable(t_id, v))
                  ->slnNeighbor()));
    }
  }

  if (score_vars.size() == 0)
    mooseError("No tallies contain the requested score " + score + "!");

  return score_vars;
}

#endif
