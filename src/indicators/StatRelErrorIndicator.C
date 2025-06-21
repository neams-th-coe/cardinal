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

#include "StatRelErrorIndicator.h"

#include "CardinalEnums.h"
#include "TallyBase.h"

registerMooseObject("CardinalApp", StatRelErrorIndicator);

InputParameters
StatRelErrorIndicator::validParams()
{
  InputParameters params = OpenMCIndicator::validParams();
  params.addClassDescription("An Indicator which reports the maximum relative error of all bins associated with a tally variable.");
  params.addRequiredParam<MooseEnum>("tally_score", getSingleTallyScoreEnum(), "The tally score used for the relative error.");

  return params;
}

StatRelErrorIndicator::StatRelErrorIndicator(const InputParameters & parameters)
  : OpenMCIndicator(parameters)
{
  std::string score = getParam<MooseEnum>("tally_score");
  std::replace(score.begin(), score.end(), '_', '-');

  if (!_openmc_problem->hasScore(score))
    paramError("tally_score",
               "The problem does not contain any score named " +
               std::string(getParam<MooseEnum>("tally_score")) +
               "! Please "
               "ensure that one of your [Tallies] is scoring the requested reaction rate.");

  if (!_openmc_problem->hasOutput(score, "rel_error"))
    mooseError("The problem does not contain any tallies that output the relative error for the score "
               + std::string(getParam<MooseEnum>("tally_score")) + "!");

  // Check to ensure the reaction rate / flux variables are CONSTANT MONOMIALS.
  bool const_mon = true;
  for (const auto v : _openmc_problem->getTallyScoreVariables(score, _tid, "_rel_error"))
    const_mon &= v->feType() == FEType(CONSTANT, MONOMIAL);

  if (!const_mon)
    paramError("tally_score",
               "StatRelErrorIndicator only supports CONSTANT MONOMIAL field variables. "
               "Please ensure your [Tallies] are adding CONSTANT MONOMIAL field variables.");

  // Grab the relative error from the [Tallies].
  _tally_rel_error = _openmc_problem->getTallyScoreVariableValues(score, _tid, "_rel_error");
}

void
StatRelErrorIndicator::computeIndicator()
{
  Real max_rel_error = 0.0;
  for (const auto & var : _tally_rel_error)
    max_rel_error = std::max((*var)[0], max_rel_error);

  _field_var.setNodalValue(max_rel_error);
}

#endif
