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

#include "OpenMCIndicator.h"

#include "libmesh/threads.h"

InputParameters
OpenMCIndicator::validParams()
{
  auto params = Indicator::validParams();
  std::vector<SubdomainName> everywhere(1, "ANY_BLOCK_ID");
  params.addParam<std::vector<SubdomainName>>(
      "block", everywhere, "block ID or name where the object works");

  return params;
}

OpenMCIndicator::OpenMCIndicator(const InputParameters & parameters)
  : Indicator(parameters),
    _openmc_problem(dynamic_cast<OpenMCCellAverageProblem *>(&_subproblem)),
    _field_var(_subproblem.getStandardVariable(_tid, name())),
    _current_elem(_field_var.currentElem())
{
  if (!_openmc_problem)
    mooseError("This indicator can only be used with problems of type 'OpenMCCellAverageProblem'!");
}

std::vector<const VariableValue *>
OpenMCIndicator::getTallyScoreVariableValues(const std::string & score)
{
  std::vector<const VariableValue *> score_vars;
  const auto & tallies = _openmc_problem->getLocalTally();
  for (const auto & t : tallies)
  {
    if (t->hasScore(score))
    {
      auto vars = t->getScoreVars(score);
      for (const auto & v : vars)
        score_vars.emplace_back(&(dynamic_cast<MooseVariableFE<Real>*>(&_subproblem.getVariable(_tid, v))->sln()));
    }
  }

  if (score_vars.size() == 0)
    mooseError("No tallies contain the requested score " + score + "!");

  return score_vars;
}

#endif
