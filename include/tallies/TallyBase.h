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

#pragma once

#include "MooseObject.h"
#include "CardinalEnums.h"

#include "openmc/tallies/tally.h"
#include "xtensor/xview.hpp"

/// Forward declarations.
class OpenMCCellAverageProblem;
class MooseMesh;
class AuxiliarySystem;
class FilterBase;

class TallyBase : public MooseObject
{
public:
  static InputParameters validParams();

  TallyBase(const InputParameters & parameters);

  /**
   * A function which the derivied tally must override to create the OpenMC spatial filter
   * the object maps to.
   * @return a pair where the first entry is the filter index in the global filter array and the
   * second entry is the OpenMC filter
   */
  virtual std::pair<unsigned int, openmc::Filter *> spatialFilter() = 0;

  /**
   * A function to initialize the tally object. Override with care.
   */
  virtual void initializeTally();

  /**
   * A function to reset the tally object. Override with care.
   */
  virtual void resetTally();

  /**
   * A function which stores the results of this tally into the created
   * auxvariables. This calls storeResultsInner.
   * @param[in] var_numbers variables which the tally will store results in
   * @param[in] local_score index into the tally's local array of scores which represents the
   * current score being stored
   * @param[in] output_type the output type
   * @return the sum of the tally over all bins. Only applicable for 'output_type = relaxed'
   */
  Real storeResults(const std::vector<unsigned int> & var_numbers,
                    unsigned int local_score,
                    const std::string & output_type);

  /**
   * Add a score to this tally.
   * @param[in] score score to add
   */
  void addScore(const std::string & score);

  /**
   * A function which sets the relaxation type and factor for a tally.
   * @param[in] relaxation_type the type of relaxation to use
   * @param[in] factor the relaxation factor to use (for constant relxation or no relaxation)
   */
  void setRelaxation(relaxation::RelaxationEnum relaxation_type, const Real & relaxation_factor);

  /**
   * A function which computes and stores the sum and mean of the tally across all bins for a
   * particular score.
   */
  void computeSumAndMean();

  /**
   * A function which gathers the sums and means from all tallies linked to this tally.
   */
  virtual void gatherLinkedSum();

  /**
   * A function which renormalizes this tally based on the gathered sum from linked tallies.
   */
  virtual void renormalizeLinkedTallies();

  /**
   * Relax the tally and normalize it according to some normalization factor 'norm'. This tends to
   * either be the sum of the over all bins OR a global tally over the entire problem.
   *
   * NOTE: This function relaxes the tally _distribution_, and not the actual magnitude of the sum.
   * That is, we relax the shape distribution and then multiply it by the power
   * (for k-eigenvalue) or source strength (for fixed source) of the current step before
   * applying it to MOOSE. If the magnitude of the power or source strength is constant in time,
   * there is zero error in this. But if the magnitude of the tally varies in time, we are basically
   * relaxing the distribution of the tally, but then multiplying it by the _current_ mean tally
   * magnitude.
   *
   * There will be very small errors in these approximations unless the power/source strength
   * change dramatically with iteration. But because relaxation is itself a numerical approximation,
   * this is still inconsequential at the end of the day as long as your problem has converged
   * the relaxed tally to the raw (unrelaxed) tally.
   */
  void relaxAndNormalizeTally();

  /**
   * Add a linked tally for normalization.
   * @param[in] tally the other tally to link for normalization
   */
  void addLinkedTally(const TallyBase * other);

  /**
   * Get the local OpenMC tally that this object wraps.
   * @return the OpenMC tally object
   */
  const openmc::Tally * getWrappedTally() const;

  /**
   * Get the global OpenMC tally that this object wraps.
   * @return the OpenMC tally object
   */
  const openmc::Tally * getWrappedGlobalTally() const;

  /**
   * Get the ID of the tally this object wraps.
   */
  int32_t getTallyID() const;

  /**
   * Get the global ID of the tally this object wraps.
   */
  int32_t getGlobalTallyID() const;

  /**
   * Get the list of scores this tally uses.
   * @return list of scores this tally uses
   */
  const std::vector<std::string> & getScores() const { return _tally_score; }

  /**
   * Get the index corresponding to a specific score.
   * @param[in] score the score
   * @return the index of the score, -1 indicates the score does not exist
   */
  int scoreIndex(const std::string & score) const;

  /**
   * Gets the auxvariable names for use in creating and storing tally results.
   * This allows for the splitting of tally results into energy bins, angular bins, etc.
   *
   * @return vector of variable names to be associated with this tally
   */
  const std::vector<std::string> & getAuxVarNames() const { return _tally_name; }

  /**
   * Gets the output names to append to the end of the '_tally_name' when adding tally auxvariables
   * for additional outputs.
   * @return additional tally outputs
   */
  const std::vector<std::string> & getOutputs() const { return _output_name; }

  /**
   * Get the estimator used in this tally.
   * @return the tally estimator
   */
  openmc::TallyEstimator getTallyEstimator() const { return _estimator; }

  /**
   * Get the mean for a score summed over all bins.
   * @param[in] local_score the index representing a tally score
   * @return mean for a score summed over all bins.
   */
  const Real & getMean(unsigned int local_score) const { return _local_mean_tally[local_score]; }

  /**
   * Get the sum for a score summed over all bins.
   * @param[in] local_score the index representing a tally score
   * @return sum for a score summed over all bins.
   */
  const Real & getSum(unsigned int local_score) const { return _local_sum_tally[local_score]; }

  /**
   * Get a vector of variable names corresponding to the provided score.
   * @param[in] score the score that the user wishes to fetch variable names from
   * @return a vector of variables corresponding to the score
   */
  std::vector<std::string> getScoreVars(const std::string & score) const;

  /**
   * Check to see if the given external filter bin is skipped during normalization.
   * @param[in] ext_bin the external filter bin
   * @return whether the bin is skipped during normalization or not
   */
  bool extBinSkipped(unsigned int ext_bin) const { return _ext_bins_to_skip[ext_bin]; }

  /**
   * Check to see if this tally uses a trigger or not.
   * @return whether this tally uses a trigger or not
   */
  bool hasTrigger() const { return _tally_trigger != nullptr; }

  /**
   * Check to see if this tally adds additional output variables or not.
   * @return whether this tally adds additional output variables or not
   */
  bool hasOutputs() const { return _has_outputs; }

  /**
   * Check to see if this tally contains a specific score.
   * @param[in] score the score to check
   * @return whether this tally has
   */
  bool hasScore(const std::string & score) const
  {
    return std::find(_tally_score.begin(), _tally_score.end(), score) != _tally_score.end();
  }

  /**
   * Check to see if the user has requested special names for the tallies.
   * @return whether this tally names stored values something other than '_tally_score'
   */
  bool renamesTallyVars() const { return _renames_tally_vars; }

  /**
   * Get the total number of external filter bins applied to this tally.
   * @return the total number of external filter bins.
   */
  unsigned int numExtFilterBins() const { return _num_ext_filter_bins; }

  /**
   * A function to get the blocks associated with this CellTally.
   * @return a set of blocks associated with this tally.
   */
  const std::set<SubdomainID> & getBlocks() const { return _tally_blocks; }

  /**
   * Whether this tally requires a global tally or not.
   * @return if the tally adds a global tally in addition to a mapped local tally
   */
  virtual bool addingGlobalTally() const { return _needs_global_tally; }

  /**
   * Get the vector of tallies linked to this tally object for normalization
   * @return the vector of linked tallies
   */
  const std::vector<const TallyBase *> & linkedTallies() const { return _linked_tallies; }

protected:
  /**
   * A function which stores the results of this tally into the created
   * auxvariables. This must be implemented by the derived class.
   * @param[in] var_numbers variables which the tally will store results in
   * @param[in] local_score index into the tally's local array of scores which represents the
   * current score being stored
   * @param[in] tally_vals the tally values to store
   * @param[in] norm_by_src_rate whether or not tally_vals should be normalized by the source rate
   * @return the sum of the tally over all bins.
   */
  virtual Real storeResultsInner(const std::vector<unsigned int> & var_numbers,
                                 unsigned int local_score,
                                 std::vector<xt::xtensor<double, 1>> tally_vals,
                                 bool norm_by_src_rate = true) = 0;

  /**
   * Set an auxiliary elemental variable to a specified value
   * @param[in] var_num variable number
   * @param[in] elem_ids element IDs to set
   * @param[in] value value to set
   */
  void fillElementalAuxVariable(const unsigned int & var_num,
                                const std::vector<unsigned int> & elem_ids,
                                const Real & value);

  /**
   * Applies triggers to a tally. This is often the local tally wrapped by this object.
   * @param[in] tally the tally to apply triggers to
   */
  void applyTriggersToLocalTally(openmc::Tally * tally);

  /**
   * Factor by which to normalize a tally
   * @param[in] score index for the tally score
   * @return value to divide tally sum by for normalization
   */
  Real tallyNormalization(unsigned int score) const;

  /**
   * Check the sum of the tallies against the global tally
   * @param[in] score tally score
   */
  void checkTallySum(const unsigned int & score) const;

  /**
   * Check that the tally normalization gives a total tally sum of 1.0 (when normalized
   * against the total tally value).
   * @param[in] sum sum of the tally
   * @param[in] score tally score
   */
  void checkNormalization(const Real & sum, unsigned int score) const;

  /// The OpenMCCellAverageProblem using the tally system.
  OpenMCCellAverageProblem & _openmc_problem;

  /// The MooseMesh.
  MooseMesh & _mesh;

  /// The aux system.
  AuxiliarySystem & _aux;

  /// The external filters added in the [Problem/Filters] block.
  std::vector<std::shared_ptr<FilterBase>> _ext_filters;

  /// The OpenMC estimator to use with this tally.
  openmc::TallyEstimator _estimator;

  /// OpenMC tally score(s) to use with this tally.
  std::vector<std::string> _tally_score;

  /// Auxiliary variable name(s) for this tally.
  std::vector<std::string> _tally_name;

  /// The OpenMC tally object this class wraps.
  openmc::Tally * _local_tally = nullptr;

  /// The index of the OpenMC tally this object wraps.
  unsigned int _local_tally_index;

  /**
   * The global OpenMC tally object this class wraps. Used for global normalization
   * and error-checking of the sum of local tally results over all bins.
   */
  openmc::Tally * _global_tally = nullptr;

  /// The index of the global OpenMC tally this object wraps.
  unsigned int _global_tally_index;

  /// The index of the first filter added by this tally.
  unsigned int _filter_index;

  /// The number of non-spatial bins in this tally.
  unsigned int _num_ext_filter_bins = 1;

  /// Sum value of this tally across all bins. Indexed by score.
  std::vector<Real> _local_sum_tally;

  /**
   * Sum value of the global tally associated with this tally object. Each entry in
   * _global_sum_tally corresponds to a score in _tally_score. As an example,
   * _global_sum_tally[0] corresponds to the score located at _tally_score[0].
   */
  std::vector<Real> _global_sum_tally;

  /**
   * Mean value of this tally across all bins; only used for fixed source mode.
   * Each entry in _local_mean_tally corresponds to a score in _tally_score. As
   * an example, _local_mean_tally[0] corresponds to the score located at
   * _tally_score[0].
   */
  std::vector<Real> _local_mean_tally;

  /// Linked sum value across all bins. Indexed by score.
  std::vector<Real> _linked_local_sum_tally;

  /**
   * Type of trigger to apply to OpenMC tallies to indicate when
   * the simulation is complete. These can be used to on-the-fly adjust the number
   * of active batches in order to reach some desired criteria (which is specified
   * by this parameter).
   */
  const MultiMooseEnum * _tally_trigger;

  /**
   * Thresholds to use for accepting this tally when using triggers. Indexed by
   * score.
   */
  std::vector<Real> _tally_trigger_threshold;

  /**
   * Whether tally bins for certain scores should ignore bins with zeros when computing
   * trigger metrics. Indexed by the tally score.
   */
  std::vector<bool> _trigger_ignore_zeros;

  /**
   * Current fixed point iteration tally result; for instance, when using constant
   * relaxation, the tally is updated as:
   * q(n+1) = (1-a) * q(n) + a * PHI(q(n), s)
   * where q(n+1) is _current_tally, a is the relaxation factor, q(n)
   * is _previous_tally, and PHI is the most-recently-computed tally result
   * (the _current_raw_tally).
   */
  std::vector<xt::xtensor<double, 1>> _current_tally;

  /// Previous fixed point iteration tally result (after relaxation)
  std::vector<xt::xtensor<double, 1>> _previous_tally;

  /// Current "raw" tally output from Monte Carlo solution
  std::vector<xt::xtensor<double, 1>> _current_raw_tally;

  /// Current "raw" tally relative error.
  std::vector<xt::xtensor<double, 1>> _current_raw_tally_rel_error;

  /// Current "raw" tally standard deviation
  std::vector<xt::xtensor<double, 1>> _current_raw_tally_std_dev;

  /**
   * How to normalize the OpenMC tally into units of W/volume. If 'true',
   * normalization is performed by dividing each local tally against a problem-global
   * tally. The advantage of this approach is that some non-zero tally regions of the
   * OpenMC domain can be excluded from multiphysics feedback (without us having to guess
   * what the power of the *included* part of the domain is). This can let us do
   * "zooming" type calculations, where perhaps we only want to send T/H feedback to
   * one bundle in a full core.
   *
   * If 'false', normalization is performed by dividing each local tally by the sum
   * of the local tally itself. The advantage of this approach becomes evident when
   * using mesh tallies. If a mesh tally does not perfectly align with an OpenMC cell -
   * for instance, a first-order sphere mesh will not perfectly match the volume of a
   * TRISO pebble - then not all of the power actually produced in the pebble is
   * tallies on the mesh approximation to that pebble. Therefore, if you set a core
   * power of 1 MW and you normalized based on a global tally, you'd always
   * miss some of that power when sending to MOOSE. So, in this case, it is better to
   * normalize against the local tally itself so that the correct power is preserved.
   */
  const bool _normalize_by_global;

  /**
   * Whether to check the tallies against the global tally;
   * if set to true, and the tallies added for the 'tally_blocks' do not
   * sum to the global tally, an error is thrown. If you are
   * only performing multiphysics feedback for, say, a single assembly in a
   * full-core OpenMC model, you must set this check to false, because there
   * are known fission sources outside the domain of interest.
   *
   * If not specified, then this is set to 'true' if normalizing by a global
   * tally, and to 'false' if normalizing by the local tally (because when we choose
   * to normalize by the local tally, we're probably using mesh tallies). But you can
   * of course still set a value for this parameter to override the default.
   */
  const bool _check_tally_sum;

  /**
   * Whether a global tally is required for the sake of normalization and/or checking
   * the tally sum
   */
  const bool _needs_global_tally;

  /// Whether this tally stores results in variables names something other than '_tally_score'.
  const bool _renames_tally_vars;

  /// Whether this tally has additional outputs or not.
  const bool _has_outputs;

  /// Suffixes to apply to 'tally_name' in order to name the fields in the 'output'.
  std::vector<std::string> _output_name;

  /// Whether the problem uses adaptive mesh refinement or not.
  const bool _is_adaptive;

  /// External filter bins to skip while computing the tally sum and mean for normalization.
  std::vector<bool> _ext_bins_to_skip;

  /// Blocks for which to add tallies.
  std::set<SubdomainID> _tally_blocks;

  /// Other tallies linked for normalization.
  std::vector<const TallyBase *> _linked_tallies;

  /// The type of relaxation this tally should employ.
  relaxation::RelaxationEnum _relaxation_type;

  /// The relaxation factor this tally should use (for constant relaxation).
  Real _relaxation_factor;

  /// Tolerance for setting zero tally
  static constexpr Real ZERO_TALLY_THRESHOLD = 1e-12;
};
