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

#include "TallyRelativeError.h"
#include "UserErrorChecking.h"
#include "xtensor/xview.hpp"

registerMooseObject("CardinalApp", TallyRelativeError);

registerMooseObjectRenamed("CardinalApp",
                           FissionTallyRelativeError,
                           "03/01/2023 24:00",
                           TallyRelativeError);

InputParameters
TallyRelativeError::validParams()
{
  InputParameters params = OpenMCPostprocessor::validParams();
  params.addParam<MooseEnum>("value_type",
                             getOperationEnum(),
                             "Whether to give the maximum or minimum tally relative error");

  params.addParam<MultiMooseEnum>(
      "tally_score",
      getTallyScoreEnum(),
      "Score to report the relative error. If there is just a single score, "
      "this defaults to that value");
  params.addClassDescription("Extract the maximum/minimum tally relative error");
  return params;
}

TallyRelativeError::TallyRelativeError(const InputParameters & parameters)
  : OpenMCPostprocessor(parameters),
    _type(getParam<MooseEnum>("value_type").getEnum<operation::OperationEnum>())
{
  auto added_scores = _openmc_problem->getTallyScores();
  if (isParamValid("tally_score"))
  {
    const auto & tally_score = getParam<MultiMooseEnum>("tally_score");
    if (tally_score.size() != 1)
      paramError(
          "tally_score",
          "Can only specify a single tally score per postprocessor, but you have specified " +
              std::to_string(tally_score.size()));

    std::string score = _openmc_problem->enumToTallyScore(tally_score[0]);

    auto it = std::find(added_scores.begin(), added_scores.end(), score);
    if (it != added_scores.end())
      _tally_index = it - added_scores.begin();
    else
      mooseError("To extract the relative error of the '" + std::string(tally_score[0]) +
                 "' score,"
                 "that score must be included in the\n'tally_score' parameter of '" +
                 _openmc_problem->type() + "'!");
  }
  else
  {
    if (added_scores.size() > 1)
      checkRequiredParam(parameters, "tally_score", "'" + _openmc_problem->type() +
        "' has more than one 'tally_score'");

    _tally_index = 0;
  }
}

Real
TallyRelativeError::getValue() const
{
  const auto & tallies = _openmc_problem->getLocalTally();

  Real post_processor_value;

  switch (_type)
  {
    case operation::max:
      post_processor_value = std::numeric_limits<Real>::min();
      break;
    case operation::min:
      post_processor_value = std::numeric_limits<Real>::max();
      break;
    case operation::average:
      post_processor_value = 0.0;
      break;
    default:
      mooseError("Unhandled OperationEnum!");
  }

  unsigned int num_values = 0;
  for (const auto & tally : tallies)
  {
    const auto t = tally->getWrappedTally();
    auto sum = xt::view(t->results_, xt::all(), _tally_index, static_cast<int>(openmc::TallyResult::SUM));
    auto sum_sq =
        xt::view(t->results_, xt::all(), _tally_index, static_cast<int>(openmc::TallyResult::SUM_SQ));

    auto rel_err = _openmc_problem->relativeError(sum, sum_sq, t->n_realizations_);
    for (int i = 0; i < t->n_filter_bins(); ++i)
    {
      // tallies without any scores to them will have zero error, which doesn't really make
      // sense to compare against
      if (MooseUtils::absoluteFuzzyEqual(sum(i), 0))
        continue;

      switch (_type)
      {
        case operation::max:
          post_processor_value = std::max(post_processor_value, rel_err[i]);
          break;
        case operation::min:
          post_processor_value = std::min(post_processor_value, rel_err[i]);
          break;
        case operation::average:
          post_processor_value += rel_err[i];
          num_values++;
          break;
        default:
          mooseError("Unhandled OperationEnum!");
      }
    }
  }

  if (_type == operation::average)
    post_processor_value /= num_values;

  return post_processor_value;
}

#endif
