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
   * Generates aux variable names for use in creating and storing tally results.
   * This allows for the splitting of tally results into energy bins, angular bins, etc.
   *
   * @return vector of variable names to be associated with this tally
   */
  virtual std::vector<std::string> generateAuxVarNames() override;

  /**
   * A function to initialize the tally which is implemented by the derived class.
   * Called by OpenMCCellAverageProblem.
   */
  virtual void initializeTally() override;

  /**
   * A function to reset the tally which is implemented by the derived class.
   * Called by OpenMCCellAverageProblem.
   */
  virtual void resetTally() override;

  /**
   * A function which stores the results of this tally into the created auxvariables.
   * This must be implemented by a derived class.
   * @return the sum of the score across all tally bins
   */
  virtual Real storeResults(const std::vector<unsigned int> & var_numbers, unsigned int score) override;

protected:
  /**
   * Loop over all the OpenMC cells and determine if a cell maps to more than one subdomain
   * that also has different tally settings (i.e. we would not know whether to add or not to
   * add tallies to the cell).
   */
  void checkCellMappedSubdomains();

  /// Blocks for which to add cell tallies.
  std::unordered_set<SubdomainID> _tally_blocks;

  /// Whether a cell index, instance pair should be added to the tally filter
  std::map<OpenMCCellAverageProblem::cellInfo, bool> _cell_has_tally;

  /// The index of the OpenMC tally this object wraps.
  unsigned int _local_tally_index;

  /// The index of the first filter added by this tally.
  unsigned int _filter_index;

  /// OpenMC mesh filter for this unstructured mesh tally.
  openmc::CellFilter * _cell_filter;
};
