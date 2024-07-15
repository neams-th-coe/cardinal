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

class TallyBase : public MooseObject
{
public:
  static InputParameters validParams();

  TallyBase(const InputParameters & parameters);

  /**
   * A function to initialize the tally which is implemented by the derived class.
   * Called by OpenMCCellAverageProblem.
   */
  virtual void initializeTally() = 0;

  /**
   * A function to reset the tally which is implemented by the derived class.
   * Called by OpenMCCellAverageProblem.
   */
  virtual void resetTally() = 0;

  /**
   * A function which stores the results of this tally into the created auxvariables.
   * This must be implemented by a derived class.
   * @param[in] var_numbers variables which the tally will store results in
   * @param[in] local_score index into the tally's local array of scores which represents the
   * current score being stored
   * @param[in] global_score index into the global array of tally results which represents the
   * current score being stored
   * @return the sum of the score across all tally bins
   */
  virtual Real storeResults(const std::vector<unsigned int> & var_numbers,
                            unsigned int local_score,
                            unsigned int global_score) = 0;

  /**
   * A function which stores the external variable results of this tally into the created
   * auxvariables. This must be implemented by a derived class.
   * @param[in] ext_var_numbers variables which the tally will store results in
   * @param[in] local_score index into the tally's local array of scores which represents the
   * current score being stored
   * @param[in] global_score index into the global array of tally results which represents the
   * current score being stored
   * @param[in] output_type the output type
   */
  virtual void storeExternalResults(const std::vector<unsigned int> & ext_var_numbers,
                                    unsigned int local_score,
                                    unsigned int global_score,
                                    const std::string & output_type) = 0;

  /**
   * Add a score to this tally.
   * @param[in] score score to add
   */
  void addScore(const std::string & score);

  /**
   * A function which computes and stores the sum and mean of the tally across all bins for a
   * particular score.
   */
  void computeSumAndMean();

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
   * @param[in] local_score the local index of the current score to normalize
   * @param[in] alpha the relaxation factor
   * @param[in] norm the normalization factor
   */
  void relaxAndNormalizeTally(unsigned int local_score, const Real & alpha, const Real & norm);

  /**
   * Get the OpenMC tally that this object wraps.
   * @return the OpenMC tally object
   */
  const openmc::Tally * getWrappedTally() const;

  /**
   * Get the list of scores this tally uses.
   * @return list of scores this tally uses
   */
  const std::vector<std::string> & getScores() const { return _tally_score; }

  /**
   * Gets the auxvariable names for use in creating and storing tally results.
   * This allows for the splitting of tally results into energy bins, angular bins, etc.
   *
   * @return vector of variable names to be associated with this tally
   */
  const std::vector<std::string> & getAuxVarNames() const { return _tally_name; }

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
   * Check to see if this tally uses a trigger or not.
   * @return whether this tally uses a trigger or not
   */
  bool hasTrigger() const { return _tally_trigger != nullptr; }

  /**
   * Check to see if the user has requested special names for the tallies.
   * @return whether this tally names stored values something other than '_tally_score'
   */
  bool renamesTallyScore() const { return isParamValid("tally_name"); }

protected:
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

  /// The OpenMCCellAverageProblem using the tally system.
  OpenMCCellAverageProblem & _openmc_problem;

  /// The MooseMesh.
  MooseMesh & _mesh;

  /// The aux system.
  AuxiliarySystem & _aux;

  /// The OpenMC estimator to use with this tally.
  openmc::TallyEstimator _estimator;

  /// OpenMC tally score(s) to use with this tally.
  std::vector<std::string> _tally_score;

  /// Auxiliary variable name(s) for this tally.
  std::vector<std::string> _tally_name;

  /// The OpenMC tally object this class wraps.
  openmc::Tally * _local_tally = nullptr;

  /// Sum value of this tally across all bins. Indexed by score.
  std::vector<Real> _local_sum_tally;

  /**
   * Mean value of this tally across all bins; only used for fixed source mode.
   * Indexed by score.
   */
  std::vector<Real> _local_mean_tally;

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

  /// Current "raw" tally standard deviation
  std::vector<xt::xtensor<double, 1>> _current_raw_tally_std_dev;

  /// Tolerance for setting zero tally
  static constexpr Real ZERO_TALLY_THRESHOLD = 1e-12;
};
