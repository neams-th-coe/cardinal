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

#include "TallyBase.h"
#include "OpenMCCellAverageProblem.h"

#include "openmc/tallies/filter_cell.h"

class CellTally : public TallyBase
{
public:
  static InputParameters validParams();

  CellTally(const InputParameters & parameters);

  /**
   * A function to generate the cell filter needed by this object.
   * @return a pair where the first entry is the filter index in the global filter array and the
   * second entry is an OpenMC distributed cell filter
   */
  virtual std::pair<unsigned int, openmc::Filter *> spatialFilter() override;

  /**
   * A function to get the blocks associated with this CellTally.
   * @return a set of blocks associated with this tally.
   */
  const std::unordered_set<SubdomainID> & getBlocks() const { return _tally_blocks; }

protected:
  /**
   * A function which stores the results of this tally into the created
   * auxvariables. This implements the distributed cell tally -> MOOSE mesh mapping.
   * @param[in] var_numbers variables which the tally will store results in
   * @param[in] local_score index into the tally's local array of scores which represents the
   * current score being stored
   * @param[in] global_score index into the global array of tally results which represents the
   * current score being stored
   * @param[in] tally_vals the tally values to store
   * @param[in] norm_by_src_rate whether or not tally_vals should be normalized by the source rate
   * @return the sum of the tally over all bins.
   */
  virtual Real storeResultsInner(const std::vector<unsigned int> & var_numbers,
                                 unsigned int local_score,
                                 unsigned int global_score,
                                 std::vector<xt::xtensor<double, 1>> tally_vals,
                                 bool norm_by_src_rate = true) override;

  /**
   * Loop over all the OpenMC cells and determine if a cell maps to more than one subdomain
   * that also has different tally settings (i.e. we would not know whether to add or not to
   * add tallies to the cell).
   */
  void checkCellMappedSubdomains();

  /**
   * Loop over all the OpenMC cells and find those for which we should add tallies.
   * @return cells to which we should add tallies
   */
  std::vector<OpenMCCellAverageProblem::cellInfo> getTallyCells() const;

  /// Blocks for which to add cell tallies.
  std::unordered_set<SubdomainID> _tally_blocks;

  /// Whether a cell index, instance pair should be added to the tally filter
  std::map<OpenMCCellAverageProblem::cellInfo, bool> _cell_has_tally;

  /// OpenMC mesh filter for this unstructured mesh tally.
  openmc::CellInstanceFilter * _cell_filter;

  /**
   * Whether to check that the [Mesh] volume each cell tally maps to is identical.
   * This is a useful helper function for OpenMC models where each cell tally has the
   * same volume (often the case for many reactor geometries). If the OpenMC model
   * cell tallies all are of the same spatial size, it's still possible that they
   * can map to different volumes in the MOOSE mesh if the MOOSE elements don't line
   * up with the edges of the OpenMC cells. Different volumes then can distort the
   * volume normalization that we do to convert the fission power to a volumetric
   * power (in a perfect world, we would actually divide OpenMC's tallies by the
   * results of a stochastic volume calculation in OpenMC, but that is too expensive).
   */
  const bool & _check_equal_mapped_tally_volumes;

  /// Absolute tolerance for checking equal tally mapped volumes
  const Real & _equal_tally_volume_abs_tol;
};
