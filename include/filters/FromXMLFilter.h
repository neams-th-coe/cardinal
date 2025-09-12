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

#include "FilterBase.h"

/**
 * A class which provides a thin wrapper around an arbitrary OpenMC
 * filter which already exists in the xml input file(s) such that
 * they can be used in a Cardinal mapped tally.
 */
class FromXMLFilter : public FilterBase
{
public:
  static InputParameters validParams();

  FromXMLFilter(const InputParameters & parameters);

  /**
   * A function which returns the short-form name for each bin of
   * this filter. Used to label auxvariables a TallyBase scores in.
   * This filter appends 'bin_index' to the value stored in '_bin_label'.
   * @param[in] bin_index the bin index
   * @return a short name for the bin represented by bin_index
   */
  virtual std::string binName(unsigned int bin_index) const override
  {
    return _bin_label + Moose::stringify(bin_index + 1);
  }

private:
  /// The OpenMC id of the filter this class should provide to tallies added by [Problem/Tallies].
  const unsigned int _filter_id;

  /// The label applied to each filter bin.
  const std::string & _bin_label;
};
