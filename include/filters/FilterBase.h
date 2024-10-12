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

#include "openmc/tallies/filter.h"

/// Forward declarations.
class OpenMCCellAverageProblem;

/**
 * A class which provides a thin wrapper around an OpenMC Filter for
 * the purposes of adding a tally via the MOOSE input syntax. This
 * class does not need init / reset functions as non-spatial tallies
 * don't change during simulation execution (at the moment).
 *
 * The filter system should NOT wrap spatial filters as the problem
 * geometry (through the MoabSkinner) will change during execution.
 * The OpenMC -> MOOSE mesh data transfer also prevents the use of
 * spatial filters in this system. Create a new tally object if a
 * different spatial filter is required.
 */
class FilterBase : public MooseObject
{
public:
  static InputParameters validParams();

  FilterBase(const InputParameters & parameters);

  /**
   * A function which returns the short-form name for each bin of
   * this filter. Used to label auxvariables a TallyBase scores in.
   * Each derived filter must override this function.
   * @param[in] the bin index
   * @return a short name for the bin represented by bin_index
   */
  virtual std::string binName(unsigned int bin_index) const = 0;

  /**
   * A function which gets the number of bins in the wrapped filter.
   * @return the number of bins in the filter
   */
  int numBins() const { return _filter->n_bins(); }

  /**
   * Get the OpenMC filter that this object wraps.
   * @return the OpenMC filter object
   */
  openmc::Filter * getWrappedFilter();

protected:
  /// The OpenMCCellAverageProblem using the tally system.
  OpenMCCellAverageProblem & _openmc_problem;

  /// The OpenMC filter this class wraps.
  openmc::Filter * _filter = nullptr;

  /// The index of the OpenMC filter this class wraps.
  unsigned int _filter_index;
};
