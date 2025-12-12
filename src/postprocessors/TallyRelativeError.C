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
  InputParameters params = GeneralPostprocessor::validParams();
  params += OpenMCBase::validParams();
  params.addParam<MooseEnum>("value_type",
                             getOperationEnum(),
                             "Whether to give the maximum or minimum tally relative error");

  params.addParam<MooseEnum>("tally_score",
                             getSingleTallyScoreEnum(),
                             "Score to report the relative error. If there is just a single score, "
                             "this defaults to that value");
  params.addParam<std::string>(
      "tally",
      "The name of the tally to fetch the score variable from. Only required if "
      "your problem contains multiple tallies which accumulate the same score.");
  params.addClassDescription("Maximum/minimum tally relative error");
  return params;
}

TallyRelativeError::TallyRelativeError(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    OpenMCBase(this, parameters),
    _type(getParam<MooseEnum>("value_type").getEnum<operation::OperationEnum>())
{
  if (isParamValid("tally_score"))
  {
    const auto & tally_score = getParam<MooseEnum>("tally_score");
    _score = _openmc_problem->enumToTallyScore(tally_score);

    if (!_openmc_problem->hasScore(_score))
      paramError(
          "tally_score",
          "To extract the relative error of the '" + std::string(tally_score) +
              "' score, it must be included in one of the [Tallies] added in your input file!");
  }
  else
  {
    if (_openmc_problem->getTallyScores().size() != 1 && !isParamValid("tally_score"))
      paramError("tally_score",
                 "When multiple scores have been added by tally objects, you must specify a score "
                 "from which the relative error will be extracted.");

    for (const auto & s : _openmc_problem->getTallyScores())
      _console << s << std::endl;

    _score = _openmc_problem->getTallyScores()[0];
  }

  // Edge case: multiple scores from linked MeshTally objects.
  const auto scoring_tallies = _openmc_problem->getNumScoringTallies(_score);
  unsigned int linked = 0;
  unsigned int num_with_score = 0;
  for (auto tally : _openmc_problem->getLocalTallies())
  {
    if (tally->hasScore(_score))
    {
      linked = std::max(linked, static_cast<unsigned int>(tally->linkedTallies().size()) + 1);
      num_with_score++;

      _tally = tally.get();
    }
  }

  if (scoring_tallies > 1)
  {
    // User must provide a tally to post-process if there isn't a single non-linked tally per score.
    if (scoring_tallies != linked || scoring_tallies != num_with_score)
    {
      // When the problem has more then one tally accumulating the given score, the user needs to
      // tell us which one to use.
      checkRequiredParam(
          _pars, "tally", "adding more then one tally with " + _score + " in the [Tallies] block");

      const auto t_name = getParam<std::string>("tally");
      _tally = _openmc_problem->getTally(t_name);
      if (!_tally)
        paramError("tally", "This tally does not exist in the [Tallies] block!");

      if (!_tally->hasScore(_score))
        paramError("tally", "This tally does not score " + _score + "!");
    }
  }
}

Real
TallyRelativeError::getValue() const
{
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

  std::vector<const TallyBase *> tallies = {_tally};
  for (const auto linked : _tally->linkedTallies())
    tallies.push_back(linked);

  unsigned int num_values = 0;
  for (const auto tally : tallies)
  {
    const auto t = tally->getWrappedTally();
    auto sum = xt::view(t->results_,
                        xt::all(),
                        tally->scoreIndex(_score),
                        static_cast<int>(openmc::TallyResult::SUM));
    auto sum_sq = xt::view(t->results_,
                           xt::all(),
                           tally->scoreIndex(_score),
                           static_cast<int>(openmc::TallyResult::SUM_SQ));

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
