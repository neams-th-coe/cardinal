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

#include "CardinalEnums.h"
#include "FilterBase.h"

class EnergyFilter : public FilterBase
{
public:
  static InputParameters validParams();

  EnergyFilter(const InputParameters & parameters);

  /**
   * A function which returns the short-form name for each bin of
   * this filter. Used to label auxvariables a TallyBase scores in.
   * EnergyFilter(s) use 'g' for each filter bin.
   * @param[in] the bin index
   * @return a short name for the bin represented by bin_index
   */
  virtual std::string binName(unsigned int bin_index) const override;

private:
  /**
   * A function which converts a GroupStructureEnum into the vector representation of the group
   * structure.
   * @param[in] structure the requested group structure
   * @return the energy gruop boundaries
   */
  std::vector<double> getGroupBoundaries(energyfilter::GroupStructureEnum group_structure);

  /// The energy bounds used to build bins.
  std::vector<Real> _energy_bnds;

  /// Whether or not to reverse the ordering of energy bins during output.
  const bool _reverse_bins;
};
