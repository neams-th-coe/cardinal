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

class SphericalHarmonicsFilter : public FilterBase
{
public:
  static InputParameters validParams();

  SphericalHarmonicsFilter(const InputParameters & parameters);

  /// Override to return false on the first bin and true on all other bins.
  virtual bool skipBin(const unsigned int bin) const override { return bin != 0; }

  /**
   * A function which returns the short-form name for each bin of
   * this filter. Used to label auxvariables a TallyBase scores in.
   * PolarAngleFilter(s) use 'theta' for each filter bin.
   * @param[in] the bin index
   * @return a short name for the bin represented by bin_index
   */
  virtual std::string binName(unsigned int bin_index) const override;

protected:
  /// The spherical harmonics order.
  const unsigned int _order;
};
