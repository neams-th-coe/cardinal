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

class FilterBase : public MooseObject
{
public:
  static InputParameters validParams();

  FilterBase(const InputParameters & parameters);

  /**
   * A function to initialize the wrapped OpenMC filter. The derived
   * tally must implement this function.
   */
  virtual void initializeFilter() = 0;

  /**
   * A function to reset the wrapped OpenMC filter. Can be overrided
   * by a derived class if required.
   */
  virtual void resetFilter();

   /**
   * A function which returns the short-form name for each bin of
   * this filter. Used to label auxvariables a TallyBase scores in.
   * The derived tally must implement this function.
   * @return a short form name for each tally bin
   */
  virtual std::string binName() const = 0;

  /**
   * A function which gets the number of bins in the wrapped filter.
   * @return the number of bins in the filter
   */
  int numBins() const { return _filter->n_bins(); }

  /**
   * Get the OpenMC filter that this object wraps.
   * @return the OpenMC filter object
   */
  const openmc::Filter * getWrappedFilter() const;

protected:
  /// The OpenMCCellAverageProblem using the tally system.
  OpenMCCellAverageProblem & _openmc_problem;

  /// The OpenMC filter this class wraps.
  openmc::Filter * _filter = nullptr;

  /// The index of the OpenMC filter this class wraps.
  unsigned int _filter_index;
};
