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

class EnergyFilter : public FilterBase
{
public:
  static InputParameters validParams();

  EnergyFilter(const InputParameters & parameters);

  virtual void initializeFilter() override;

  /**
   * A function which returns the short-form name for each bin of
   * this filter. Used to label auxvariables a TallyBase scores in.
   * EnergyFilter(s) append 'g' for each filter bin.
   * @return a short form name for each tally bin
   */
  virtual std::string binName() const { return "g"; }

private:
  /// The energy bounds used to build bins.
  std::vector<Real> _energy_bnds;
};
