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
#include "UserErrorChecking.h"
#include "AuxiliarySystem.h"
#include "FilterBase.h"

#include "openmc/settings.h"

InputParameters
TallyBase::validParams()
{
  auto params = MooseObject::validParams();
  params.addParam<MultiMooseEnum>(
      "score",
      getTallyScoreEnum(),
      "Score(s) to use in the OpenMC tallies. If not specified, defaults to 'kappa_fission'");
  params.addParam<MooseEnum>(
      "estimator", getTallyEstimatorEnum(), "Type of tally estimator to use in OpenMC");
  params.addParam<std::vector<std::string>>(
      "name",
      "Auxiliary variable name(s) to use for OpenMC tallies. "
      "If not specified, defaults to the names of the scores");

  MultiMooseEnum tally_trigger("rel_err none");
  params.addParam<MultiMooseEnum>(
      "trigger",
      tally_trigger,
      "Trigger criterion to determine when OpenMC simulation is complete "
      "based on tallies. If multiple scores are specified in 'score, "
      "this same trigger is applied to all scores.");
  params.addRangeCheckedParam<std::vector<Real>>(
      "trigger_threshold", "trigger_threshold > 0", "Threshold for the tally trigger");
  params.addParam<std::vector<bool>>(
      "trigger_ignore_zeros",
      {false},
      "Whether tally bins with zero scores are ignored when computing the tally trigger. If only "
      "one "
      "value of 'trigger_ignore_zeros' is provided, that value is applied to all tally scores.");

  MultiMooseEnum openmc_outputs("unrelaxed_tally_std_dev unrelaxed_tally");
  params.addParam<MultiMooseEnum>("output",
                                  openmc_outputs,
                                  "UNRELAXED field(s) to output from OpenMC for each tally score. "
                                  "unrelaxed_tally_std_dev will write the standard deviation of "
                                  "each tally into auxiliary variables "
                                  "named *_std_dev. Unrelaxed_tally will write the raw unrelaxed "
                                  "tally into auxiliary variables "
                                  "named *_raw (replace * with 'name').");

  params.addParam<std::vector<std::string>>("filters", "External filters to add to this tally.");

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
    _tally_trigger(isParamValid("trigger") ? &getParam<MultiMooseEnum>("trigger") : nullptr),
    _trigger_ignore_zeros(getParam<std::vector<bool>>("trigger_ignore_zeros")),
    _renames_tally_vars(isParamValid("name")),
    _has_outputs(isParamValid("output")),
    _is_adaptive(_openmc_problem.hasAdaptivity())
{
  if (isParamValid("score"))
  {
    const auto & scores = getParam<MultiMooseEnum>("score");
    for (const auto & score : scores)
      _tally_score.push_back(_openmc_problem.enumToTallyScore(score));
  }
  else
    _tally_score = {"kappa-fission"};

  bool heating =
      std::find(_tally_score.begin(), _tally_score.end(), "heating") != _tally_score.end();

  if (isParamValid("estimator"))
  {
    auto estimator = getParam<MooseEnum>("estimator").getEnum<tally::TallyEstimatorEnum>();

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

  if (isParamValid("trigger") != isParamValid("trigger_threshold"))
    mooseError("You must either specify none or both of 'trigger' and "
               "'trigger_threshold'. You have specified only one.");

  if (_tally_trigger)
  {
    checkRequiredParam(parameters, "trigger_threshold", "using tally triggers");
    _tally_trigger_threshold = getParam<std::vector<Real>>("trigger_threshold");

    if (_tally_trigger->size() != _tally_score.size())
      mooseError("'trigger' (size " + std::to_string(_tally_trigger->size()) +
                 ") must have the same length as 'score' (size " +
                 std::to_string(_tally_score.size()) + ")");

    if (_tally_trigger_threshold.size() != _tally_score.size())
      mooseError("'trigger_threshold' (size " + std::to_string(_tally_trigger_threshold.size()) +
                 ") must have the same length as 'score' (size " +
                 std::to_string(_tally_score.size()) + ")");

    if (_trigger_ignore_zeros.size() > 1)
    {
      if (_tally_score.size() != _trigger_ignore_zeros.size())
        mooseError("'trigger_ignore_zeros' (size " + std::to_string(_trigger_ignore_zeros.size()) +
                   ") must have the same length as 'score' (size " +
                   std::to_string(_tally_score.size()) + ")");
    }
    else if (_trigger_ignore_zeros.size() == 1)
      _trigger_ignore_zeros.resize(_tally_score.size(), _trigger_ignore_zeros[0]);

    _openmc_problem.checkEmptyVector(_trigger_ignore_zeros, "trigger_ignore_zeros");
  }

  // Fetch the filters required by this tally. Error if the filter hasn't been added yet.
  if (isParamValid("filters"))
  {
    for (const auto & filter_name : getParam<std::vector<std::string>>("filters"))
    {
      if (!_openmc_problem.hasFilter(filter_name))
        mooseError("Filter with the name " + filter_name + " does not exist!");

      _ext_filters.push_back(_openmc_problem.getFilter(filter_name));
    }
  }

  if (isParamValid("name"))
    _tally_name = getParam<std::vector<std::string>>("name");
  else
  {
    for (auto score : _tally_score)
    {
      std::replace(score.begin(), score.end(), '-', '_');
      _tally_name.push_back(score);
    }
  }

  if (_has_outputs)
  {
    // names of output are appended to ends of 'name'
    for (const auto & o : getParam<MultiMooseEnum>("output"))
    {
      std::string name = o;

      if (o == "UNRELAXED_TALLY_STD_DEV")
        _output_name.push_back("std_dev");
      else if (o == "UNRELAXED_TALLY")
        _output_name.push_back("raw");
      else
        mooseError("Unhandled OutputEnum in OpenMCCellAverageProblem!");
    }
  }

  if (_tally_name.size() != _tally_score.size())
    mooseError("'name' must be the same length as 'score'!");

  // Modify the variable names so they take into account the bins in the external filters.
  auto all_var_names = _tally_name;
  for (const auto & filter : _ext_filters)
  {
    std::vector<std::string> n;
    for (unsigned int i = 0; i < all_var_names.size(); ++i)
      for (unsigned int j = 0; j < filter->numBins(); ++j)
        n.push_back(all_var_names[i] + "_" + filter->binName(j));

    all_var_names = n;

    _num_ext_filter_bins *= filter->numBins();
  }
  _tally_name = all_var_names;

  _openmc_problem.checkDuplicateEntries(_tally_name, "name");
  _openmc_problem.checkDuplicateEntries(_tally_score, "score");

  _local_sum_tally.resize(_tally_score.size(), 0.0);
  _local_mean_tally.resize(_tally_score.size(), 0.0);

  _current_tally.resize(_tally_score.size());
  _current_raw_tally.resize(_tally_score.size());
  _current_raw_tally_std_dev.resize(_tally_score.size());
  _previous_tally.resize(_tally_score.size());
}

void
TallyBase::initializeTally()
{
  // Clear cached results.
  _local_sum_tally.clear();
  _local_sum_tally.resize(_tally_score.size(), 0.0);
  _local_mean_tally.clear();
  _local_mean_tally.resize(_tally_score.size(), 0.0);

  _current_tally.resize(_tally_score.size());
  _current_raw_tally.resize(_tally_score.size());
  _current_raw_tally_std_dev.resize(_tally_score.size());
  _previous_tally.resize(_tally_score.size());

  auto [index, spatial_filter] = spatialFilter();
  _filter_index = index;

  std::vector<openmc::Filter *> filters;
  for (auto & filter : _ext_filters)
    filters.push_back(filter->getWrappedFilter());
  // We add the spatial filter last to minimize the number of cache
  // misses during the OpenMC -> Cardinal transfer.
  filters.push_back(spatial_filter);

  // Create the tally, assign the required filters and apply the triggers.
  _local_tally_index = openmc::model::tallies.size();
  _local_tally = openmc::Tally::create();
  _local_tally->set_scores(_tally_score);
  _local_tally->estimator_ = _estimator;
  _local_tally->set_filters(filters);
  applyTriggersToLocalTally(_local_tally);
}

void
TallyBase::resetTally()
{
  // Erase the tally.
  openmc::model::tallies.erase(openmc::model::tallies.begin() + _local_tally_index);

  // Erase the filter(s).
  openmc::model::tally_filters.erase(openmc::model::tally_filters.begin() + _filter_index);
}

Real
TallyBase::storeResults(const std::vector<unsigned int> & var_numbers,
                        unsigned int local_score,
                        unsigned int global_score,
                        const std::string & output_type)
{
  Real total = 0.0;

  if (output_type == "relaxed")
    total += storeResultsInner(var_numbers, local_score, global_score, _current_tally);
  else if (output_type == "std_dev")
    storeResultsInner(var_numbers, local_score, global_score, _current_raw_tally_std_dev);
  else if (output_type == "raw")
    storeResultsInner(var_numbers, local_score, global_score, _current_raw_tally);
  else
    mooseError("Unknown external output " + output_type);

  return total;
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
                                  _trigger_ignore_zeros[score],
                                  score});
}
#endif
