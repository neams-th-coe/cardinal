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
 * A class which provides a thin wrapper around an OpenMC LegendreFilter
 * for use by Cardinal mapped tallies.
 */
class AngularLegendreFilter : public FilterBase
{
public:
  static InputParameters validParams();

  AngularLegendreFilter(const InputParameters & parameters);

  /// Override to return false on the first bin and true on all other bins.
  virtual bool skipBin(const unsigned int bin) const override { return bin != 0; }

  /**
   * A function which returns the short-form name for each bin of
   * this filter. Used to label auxvariables a TallyBase scores in.
   * AngularLegendreFilter(s) use l.
   * @param[in] bin_index the bin index
   * @return a short name for the bin represented by bin_index
   */
  virtual std::string binName(unsigned int bin_index) const override;

private:
  /// The Legendre order.
  const unsigned int _order;
};
