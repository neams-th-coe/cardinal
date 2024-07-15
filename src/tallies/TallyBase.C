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
#include "TallyBase.h"

#include "OpenMCCellAverageProblem.h"
#include "AuxiliarySystem.h"

#include "openmc/settings.h"

InputParameters
TallyBase::validParams()
{
  auto params = MooseObject::validParams();
  params.addParam<MultiMooseEnum>(
      "tally_score",
      getTallyScoreEnum(),
      "Score(s) to use in the OpenMC tallies. If not specified, defaults to 'kappa_fission'");
  params.addParam<MooseEnum>(
      "tally_estimator", getTallyEstimatorEnum(), "Type of tally estimator to use in OpenMC");
  params.addParam<std::vector<std::string>>(
      "tally_name",
      "Auxiliary variable name(s) to use for OpenMC tallies. "
      "If not specified, defaults to the names of the scores");

  MultiMooseEnum tally_trigger("rel_err none");
  params.addParam<MultiMooseEnum>(
      "tally_trigger",
      tally_trigger,
      "Trigger criterion to determine when OpenMC simulation is complete "
      "based on tallies. If multiple scores are specified in 'tally_score, "
      "this same trigger is applied to all scores.");
  params.addRangeCheckedParam<std::vector<Real>>(
      "tally_trigger_threshold", "tally_trigger_threshold > 0", "Threshold for the tally trigger");

  params.addPrivateParam<OpenMCCellAverageProblem *>("_openmc_problem");

  params.registerBase("Tally");
  params.registerSystemAttributeName("Tally");

  return params;
}

TallyBase::TallyBase(const InputParameters & parameters)
  : MooseObject(parameters),
    _openmc_problem(*getParam<OpenMCCellAverageProblem *>("_openmc_problem")),
    _mesh(_openmc_problem.mesh()),
    _aux(_openmc_problem.getAuxiliarySystem()),
    _tally_trigger(isParamValid("tally_trigger") ? &getParam<MultiMooseEnum>("tally_trigger")
                                                 : nullptr)
{
  if (isParamValid("tally_score"))
  {
    const auto & scores = getParam<MultiMooseEnum>("tally_score");
    for (const auto & score : scores)
      _tally_score.push_back(_openmc_problem.enumToTallyScore(score));
  }
  else
    _tally_score = {"kappa-fission"};

  bool heating =
      std::find(_tally_score.begin(), _tally_score.end(), "heating") != _tally_score.end();

  if (isParamValid("tally_estimator"))
  {
    auto estimator = getParam<MooseEnum>("tally_estimator").getEnum<tally::TallyEstimatorEnum>();

    // Photon heating tallies cannot use tracklength estimators.
    if (estimator == tally::tracklength && openmc::settings::photon_transport && heating)
      mooseError("Tracklength estimators are currently incompatible with photon transport and "
                 "heating scores! For more information: https://tinyurl.com/3wre3kwt");

    _estimator = _openmc_problem.tallyEstimator(estimator);
  }
  else
  {
    /**
     * Set a default of tracklength for all tallies other then heating tallies in photon transport.
     * This behavior must be overridden in derived tallies that implement mesh filters.
     */
    _estimator = openmc::TallyEstimator::TRACKLENGTH;
    if (heating && openmc::settings::photon_transport)
      _estimator = openmc::TallyEstimator::COLLISION;
  }

  if (heating && !openmc::settings::photon_transport)
    mooseWarning(
        "When using the 'heating' score with photon transport disabled, energy deposition\n"
        "from photons is neglected unless you specifically ran NJOY to produce MT=301 with\n"
        "photon energy deposited locally (not true for any pre-packaged OpenMC data libraries\n"
        "on openmc.org).\n\n"
        "If you did NOT specifically run NJOY yourself with this customization, we recommend\n"
        "using the 'heating_local' score instead, which will capture photon energy deposition.\n"
        "Otherwise, you will underpredict the true energy deposition.");

  if (isParamValid("tally_trigger") != isParamValid("tally_trigger_threshold"))
    mooseError("You must either specify none or both of 'tally_trigger' and "
               "'tally_trigger_threshold'. You have specified only one.");

  bool has_tally_trigger = false;
  if (_tally_trigger)
  {
    _tally_trigger_threshold = getParam<std::vector<Real>>("tally_trigger_threshold");

    if (_tally_trigger->size() != _tally_score.size())
      mooseError("'tally_trigger' (size " + std::to_string(_tally_trigger->size()) +
                 ") must have the same length as 'tally_score' (size " +
                 std::to_string(_tally_score.size()) + ")");

    if (_tally_trigger_threshold.size() != _tally_score.size())
      mooseError("'tally_trigger_threshold' (size " +
                 std::to_string(_tally_trigger_threshold.size()) +
                 ") must have the same length as 'tally_score' (size " +
                 std::to_string(_tally_score.size()) + ")");

    for (unsigned int s = 0; s < _tally_trigger->size(); ++s)
      if ((*_tally_trigger)[s] != "none")
        has_tally_trigger = true;
  }

  if (isParamValid("tally_name"))
    _tally_name = getParam<std::vector<std::string>>("tally_name");
  else
  {
    for (auto score : _tally_score)
    {
      std::replace(score.begin(), score.end(), '-', '_');
      _tally_name.push_back(score);
    }
  }

  if (_tally_name.size() != _tally_score.size())
    mooseError("'tally_name' must be the same length as 'tally_score'!");

  _openmc_problem.checkDuplicateEntries(_tally_name, "tally_name");
  _openmc_problem.checkDuplicateEntries(_tally_score, "tally_score");

  _local_sum_tally.resize(_tally_score.size(), 0.0);
  _local_mean_tally.resize(_tally_score.size(), 0.0);

  _current_tally.resize(_tally_score.size());
  _current_raw_tally.resize(_tally_score.size());
  _current_raw_tally_std_dev.resize(_tally_score.size());
  _previous_tally.resize(_tally_score.size());
}

void
TallyBase::addScore(const std::string & score)
{
  _tally_score.push_back(score);

  std::string s = score;
  std::replace(s.begin(), s.end(), '-', '_');
  _tally_name.push_back(s);

  _local_sum_tally.resize(_tally_score.size(), 0.0);
  _local_mean_tally.resize(_tally_score.size(), 0.0);

  _current_tally.resize(_tally_score.size());
  _current_raw_tally.resize(_tally_score.size());
  _current_raw_tally_std_dev.resize(_tally_score.size());
  _previous_tally.resize(_tally_score.size());
}

void
TallyBase::computeSumAndMean()
{
  for (unsigned int score = 0; score < _tally_score.size(); ++score)
  {
    _local_sum_tally[score] = _openmc_problem.tallySumAcrossBins({_local_tally}, score);
    _local_mean_tally[score] = _openmc_problem.tallyMeanAcrossBins({_local_tally}, score);
  }
}

void
TallyBase::relaxAndNormalizeTally(unsigned int local_score, const Real & alpha, const Real & norm)
{
  auto & current = _current_tally[local_score];
  auto & previous = _previous_tally[local_score];
  auto & current_raw = _current_raw_tally[local_score];
  auto & current_raw_std_dev = _current_raw_tally_std_dev[local_score];

  auto mean_tally = _openmc_problem.tallySum(_local_tally, local_score);
  /**
   * If the value over the whole domain is zero, then the values in the individual bins must be
   * zero. We need to avoid divide-by-zeros.
   */
  current_raw = std::abs(norm) < ZERO_TALLY_THRESHOLD
                    ? static_cast<xt::xtensor<double, 1>>(mean_tally * 0.0)
                    : static_cast<xt::xtensor<double, 1>>(mean_tally / norm);

  auto sum_sq = xt::view(_local_tally->results_,
                         xt::all(),
                         local_score,
                         static_cast<int>(openmc::TallyResult::SUM_SQ));
  auto rel_err = _openmc_problem.relativeError(mean_tally, sum_sq, _local_tally->n_realizations_);
  current_raw_std_dev = rel_err * current_raw;

  if (_openmc_problem.fixedPointIteration() == 0 || alpha == 1.0)
  {
    current = current_raw;
    previous = current_raw;
    return;
  }

  // Save the current tally (from the previous iteration) into the previous one.
  std::copy(current.cbegin(), current.cend(), previous.begin());

  // Relax the tallies by alpha. TODO: skip relaxation when alpha is one.
  auto relaxed_tally = (1.0 - alpha) * previous + alpha * current_raw;
  std::copy(relaxed_tally.cbegin(), relaxed_tally.cend(), current.begin());
}

const openmc::Tally *
TallyBase::getWrappedTally() const
{
  if (!_local_tally)
    mooseError("This tally has not been initialized!");

  return _local_tally;
}

void
TallyBase::fillElementalAuxVariable(const unsigned int & var_num,
                                    const std::vector<unsigned int> & elem_ids,
                                    const Real & value)
{
  auto & solution = _aux.solution();
  auto sys_number = _aux.number();

  // loop over all the elements and set the specified variable to the specified value
  for (const auto & e : elem_ids)
  {
    auto elem_ptr = _mesh.queryElemPtr(e);

    if (!_openmc_problem.isLocalElem(elem_ptr))
      continue;

    auto dof_idx = elem_ptr->dof_number(sys_number, var_num, 0);
    solution.set(dof_idx, value);
  }
}

void
TallyBase::applyTriggersToLocalTally(openmc::Tally * tally)
{
  if (_tally_trigger)
    for (int score = 0; score < _tally_score.size(); ++score)
      tally->triggers_.push_back({_openmc_problem.triggerMetric((*_tally_trigger)[score]),
                                  _tally_trigger_threshold[score],
                                  false,
                                  score});
}
#endif
