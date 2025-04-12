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

#ifdef ENABLE_OPENMC_COUPLING
#include "EnergyOutFilter.h"

#include "EnergyGroupStructures.h"
#include "EnergyFilter.h"
#include "CardinalEnums.h"

#include "openmc/tallies/filter_energy.h"

registerMooseObject("CardinalApp", EnergyOutFilter);

InputParameters
EnergyOutFilter::validParams()
{
  auto params = FilterBase::validParams();
  params += EnergyBinBase::validParams();
  params.addClassDescription(
      "A class which provides a thin wrapper around an OpenMC EnergyOutFilter. Energy bins "
      "can either be manually specified in 'energy_boundaries' or picked from a list "
      "provided in 'group_structure'.");

  return params;
}

EnergyOutFilter::EnergyOutFilter(const InputParameters & parameters)
  : FilterBase(parameters),
    EnergyBinBase(this, parameters)
{
  // Initialize the OpenMC EnergyoutFilter.
  _filter_index = openmc::model::tally_filters.size();

  auto energy_out_filter =
      dynamic_cast<openmc::EnergyoutFilter *>(openmc::Filter::create("energyout"));
  energy_out_filter->set_bins(_energy_bnds);
  _filter = energy_out_filter;
}

std::string
EnergyOutFilter::binName(unsigned int bin_index) const
{
  return "gp" + (_reverse_bins ? Moose::stringify(_energy_bnds.size() - bin_index - 1)
                               : Moose::stringify(bin_index + 1));
}

#endif
