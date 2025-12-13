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

#include "TallyInterface.h"

#include "UserErrorChecking.h"

InputParameters
TallyInterface::validParams()
{
  InputParameters params = OpenMCBase::validParams();
  return params;
}

TallyInterface::TallyInterface(const ParallelParamObject * object, const InputParameters & parameters)
  : OpenMCBase(object, parameters),
    _object(object)
{
}

std::string
TallyInterface::getScore(const std::string & score_param)
{
  std::string score = _object->getParam<MooseEnum>(score_param);
  std::replace(score.begin(), score.end(), '_', '-');

  if (!_openmc_problem->hasScore(score))
    _object->paramError(score_param,
                        "The problem does not contain any score named " +
                        std::string(_object->getParam<MooseEnum>(score_param)) +
                        "! Please ensure that one of your [Tallies] is "
                        "accumulating the requested score.");

  return score;
}

std::string
TallyInterface::tallyByScore(const std::string & score, const std::string & tally_param)
{
  if (_openmc_problem->getNumScoringTallies(score) == 0)
    _object->mooseError("No tallies are adding a " + score + " score!");

  // When the problem has more then one tally accumulating the given score, the user needs to tell
  // us which one to use.
  std::string tally_name;
  if (_openmc_problem->getNumScoringTallies(score) > 1)
  {
    checkRequiredParam(_object->parameters(),
                        tally_param,
                        "adding more then one tally with " + score + " in the [Tallies] block");

    tally_name = _object->getParam<std::string>(tally_param);
    const auto * tally = _openmc_problem->getTally(tally_name);
    if (!tally)
      _object->paramError(tally_param, "This tally does not exist in the [Tallies] block!");

    if (!tally->hasScore(score))
      _object->paramError(tally_param, "This tally does not score " + score + "!");
  }
  else
  {
    const auto & all_tallies = _openmc_problem->getLocalTallies();
    for (const auto & t : all_tallies)
      if (t->hasScore(score))
        tally_name = t->name();
  }

  return tally_name;
}
