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
#include "EnergyFilter.h"
#include "EnergyGroupStructures.h"

#include "openmc/tallies/filter_energy.h"

registerMooseObject("CardinalApp", EnergyFilter);

InputParameters
EnergyFilter::validParams()
{
  auto params = FilterBase::validParams();
  params.addClassDescription(
      "A class which provides a thin wrapper around an OpenMC EnergyFilter. Energy bins "
      "can either be manually specified in 'energy_boundaries' or picked from a list "
      "provided in 'group_structure'.");
  params.addParam<std::vector<Real>>(
      "energy_boundaries",
      "The energy boundaries to use to form energy bins. The boundaries must be provided "
      "in ascending order in terms of energy.");
  params.addParam<MooseEnum>(
      "group_structure",
      getEnergyGroupsEnum(),
      "The energy group structure to use from a list of popular group structures.");
  params.addParam<bool>(
      "reverse_bins",
      false,
      "Whether the bins should be output in reverse order. This is useful for comparing "
      "deterministic transport codes with Cardinal, where the convention is that energy "
      "groups are arranged in descending order (i.e. group 1 is fast, group 2 is thermal).");

  return params;
}

EnergyFilter::EnergyFilter(const InputParameters & parameters)
  : FilterBase(parameters), _reverse_bins(getParam<bool>("reverse_bins"))
{
  if (isParamValid("energy_boundaries") == isParamValid("group_structure"))
    mooseError("You have either set both 'energy_boundaries' and 'group_structure' or have not "
               "specified a bin option. Please specify either 'energy_boundaries' or "
               "'group_structure'.");

  if (isParamValid("energy_boundaries"))
    _energy_bnds = getParam<std::vector<Real>>("energy_boundaries");
  if (isParamValid("group_structure"))
    _energy_bnds = getGroupBoundaries(
        getParam<MooseEnum>("group_structure").getEnum<energyfilter::GroupStructureEnum>());

  // Two boundaries are required at minimum to form energy bins.
  if (_energy_bnds.size() < 2)
    paramError("energy_boundaries",
               "At least two energy values are required to create energy bins!");

  // Sort the boundaries so they're monotonically decreasing.
  std::sort(_energy_bnds.begin(),
            _energy_bnds.end(),
            [](const Real & a, const Real & b) { return a < b; });

  // Initialize the OpenMC EnergyFilter.
  _filter_index = openmc::model::tally_filters.size();

  auto energy_filter = dynamic_cast<openmc::EnergyFilter *>(openmc::Filter::create("energy"));
  energy_filter->set_bins(_energy_bnds);
  _filter = energy_filter;
}

std::string
EnergyFilter::binName(unsigned int bin_index) const
{
  return "g" + (_reverse_bins ? Moose::stringify(_energy_bnds.size() - bin_index - 1)
                              : Moose::stringify(bin_index + 1));
}

std::vector<double>
EnergyFilter::getGroupBoundaries(energyfilter::GroupStructureEnum group_structure)
{
  using namespace energyfilter;
  using namespace groupstructures;
  switch (group_structure)
  {
    case CASMO_2:
      return std::vector<double>(S_CASMO_2, S_CASMO_2 + 3);
    case CASMO_4:
      return std::vector<double>(S_CASMO_4, S_CASMO_4 + 5);
    case CASMO_8:
      return std::vector<double>(S_CASMO_8, S_CASMO_8 + 9);
    case CASMO_16:
      return std::vector<double>(S_CASMO_16, S_CASMO_16 + 17);
    case CASMO_25:
      return std::vector<double>(S_CASMO_25, S_CASMO_25 + 26);
    case CASMO_40:
      return std::vector<double>(S_CASMO_40, S_CASMO_40 + 41);
    case VITAMINJ_42:
      return std::vector<double>(S_VITAMINJ_42, S_VITAMINJ_42 + 43);
    case SCALE_44:
      return std::vector<double>(S_SCALE_44, S_SCALE_44 + 45);
    case MPACT_51:
      return std::vector<double>(S_MPACT_51, S_MPACT_51 + 52);
    case MPACT_60:
      return std::vector<double>(S_MPACT_60, S_MPACT_60 + 61);
    case MPACT_69:
      return std::vector<double>(S_MPACT_69, S_MPACT_69 + 70);
    case CASMO_70:
      return std::vector<double>(S_CASMO_70, S_CASMO_70 + 71);
    case XMAS_172:
      return std::vector<double>(S_XMAS_172, S_XMAS_172 + 173);
    case VITAMINJ_175:
      return std::vector<double>(S_VITAMINJ_175, S_VITAMINJ_175 + 176);
    case SCALE_252:
      return std::vector<double>(S_SCALE_252, S_SCALE_252 + 253);
    case TRIPOLI_315:
      return std::vector<double>(S_TRIPOLI_315, S_TRIPOLI_315 + 316);
    case SHEM_361:
      return std::vector<double>(S_SHEM_361, S_SHEM_361 + 362);
    case CCFE_709:
      return std::vector<double>(S_CCFE_709, S_CCFE_709 + 710);
    case UKAEA_1102:
      return std::vector<double>(S_UKAEA_1102, S_UKAEA_1102 + 1103);
    case ECCO_1968:
      return std::vector<double>(S_ECCO_1968, S_ECCO_1968 + 1969);
    default:
      mooseError("Unknown energy group structure!");
  }
  return std::vector<double>();
}
#endif
