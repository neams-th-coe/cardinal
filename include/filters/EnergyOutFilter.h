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
#include "EnergyBinBase.h"

/**
 * A class which provides a thin wrapper around an OpenMC EnergyoutFilter
 * for use by Cardinal mapped tallies.
 */
class EnergyOutFilter : public FilterBase, public EnergyBinBase
{
public:
  static InputParameters validParams();

  EnergyOutFilter(const InputParameters & parameters);

  /**
   * A function which returns the short-form name for each bin of
   * this filter. Used to label auxvariables a TallyBase scores in.
   * EnergyOutFilter(s) use 'gp' for each filter bin.
   * @param[in] bin_index the bin index
   * @return a short name for the bin represented by bin_index
   */
  virtual std::string binName(unsigned int bin_index) const override;
};
