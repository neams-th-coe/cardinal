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
  params.addClassDescription("An Indicator which reports the maximum relative error of all bins "
                             "associated with a tally variable.");
  params.addRequiredParam<MooseEnum>(
      "score", getSingleTallyScoreEnum(), "The tally score used for the relative error.");
  params.addParam<unsigned int>(
      "ext_filter_bin",
      0,
      "The non-spatial filter bin for the tally score (with bin indices starting at 0).");

  return params;
}

StatRelErrorIndicator::StatRelErrorIndicator(const InputParameters & parameters)
  : OpenMCIndicator(parameters), _bin_index(getParam<unsigned int>("ext_filter_bin"))
{
  std::string score = getParam<MooseEnum>("score");
  std::replace(score.begin(), score.end(), '_', '-');

  if (!_openmc_problem->hasScore(score))
    paramError("score",
               "The problem does not contain any score named " +
                   std::string(getParam<MooseEnum>("score")) +
                   "! Please "
                   "ensure that one of your [Tallies] is scoring the requested reaction rate.");

  if (!_openmc_problem->hasOutput(score, "rel_error"))
    mooseError(
        "The problem does not contain any tallies that output the relative error for the score " +
        std::string(getParam<MooseEnum>("score")) + "!");

  // Check to ensure the reaction rate / flux variables are CONSTANT MONOMIALS.
  bool const_mon = true;
  for (const auto v : _openmc_problem->getTallyScoreVariables(score, _tid, "_rel_error"))
    const_mon &= v->feType() == FEType(CONSTANT, MONOMIAL);

  if (!const_mon)
    paramError("score",
               "StatRelErrorIndicator only supports CONSTANT MONOMIAL field variables. "
               "Please ensure your [Tallies] are adding CONSTANT MONOMIAL field variables.");

  // Grab the relative error from the [Tallies].
  const auto score_bins = _openmc_problem->getTallyScoreVariableValues(score, _tid, "_rel_error");
  if (_bin_index >= score_bins.size())
    paramError("ext_filter_bin",
               "The external filter bin provided is invalid for the number of "
               "external filter bins (" +
                   std::to_string(score_bins.size()) +
                   ") "
                   "applied to " +
                   std::string(getParam<MooseEnum>("score")) + "!");

  _tally_rel_error = score_bins[_bin_index];
}

void
StatRelErrorIndicator::computeIndicator()
{
  _field_var.setNodalValue((*_tally_rel_error)[0]);
}

#endif
