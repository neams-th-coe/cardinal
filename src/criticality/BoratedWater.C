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

#define NUCLEAR_DATA_IMPLEMENTATION

#include "BoratedWater.h"
#include "UserErrorChecking.h"
#include "NuclearData.h"
#include "openmc/capi.h"
#include "openmc/constants.h"
#include "openmc/nuclide.h"

registerMooseObject("CardinalApp", BoratedWater);

InputParameters
BoratedWater::validParams()
{
  auto params = OpenMCMaterialSearch::validParams();
  params.addParam<std::vector<std::string>>(
      "absent_nuclides",
      "Natural nuclides of hydrogen, oxygen, or boron which are missing from your cross section "
      "library; some cross section libraries do not have entries for O17 and O18. If your library "
      "does not have these nuclides you will get an error from this object trying to add them. For "
      "these missing nuclides, specify them here and their abundance will be applied to the main "
      "isotope of each element. Currently, only O18 is supported.");
  params.addClassDescription(
      "Searches for criticality using natural boron ppm in water in units of weight ppm");
  return params;
}

BoratedWater::BoratedWater(const InputParameters & parameters) : OpenMCMaterialSearch(parameters)
{
  // apply additional checks on the minimum and maximum
  if (_minimum < 0)
    paramError("minimum",
               "The 'minimum' boron ppm (" + std::to_string(_minimum) + ") must be positive!");
  if (_maximum > 50000)
    paramError("maximum",
               "The borated water composition is computed using a dilute species approximation. "
               "Results will not be accurate if the boron species is no longer dilute, which we "
               "take as 5\% weight concentration or higher. Please decrease 'maximum' (" +
                   std::to_string(_maximum) + ") to an upper limit which is in the dilute regime.");

  // We take the provided material, retaining its density, but then overwriting
  // any nuclides there to be H2O + boron weight ppm and add the S(a,b) tables
  // for hydrogen. Therefore, we perform a check here if any nuclides in the
  // material are *not* those we are going to add back when we wipe the material,
  // to notify the user they probably provided the wrong material ID or they need
  // to use a more general material modification object that allows full control
  // over additional nuclides (e.g., if their water includes corrosion products
  // that they do want to be there).

  const int * nuclides;
  const double * densities;
  int n;
  int err = openmc_material_get_densities(_material_index, &nuclides, &densities, &n);
  catchOpenMCError(err, "get nuclide densities from material " + std::to_string(_material_id));

  // get all the natural isotopes of H, O, B
  _hydrogen_natural = NuclearData::Nuclide::getAbundances("H");
  _boron_natural = NuclearData::Nuclide::getAbundances("B");
  _oxygen_natural = NuclearData::Nuclide::getAbundances("O");

  std::vector<std::string> allowable;
  for (const auto & i : _hydrogen_natural)
    allowable.push_back(i.first);
  for (const auto & i : _boron_natural)
    allowable.push_back(i.first);
  for (const auto & i : _oxygen_natural)
    allowable.push_back(i.first);

  applyAbsentNuclides(allowable);

  // check if any nuclides already defined on the material do not intersect with
  // natural isotopes of H, O, B
  std::string full_names = "";
  for (int i = 0; i < n; ++i)
  {
    std::string name = openmc::data::nuclides[i]->name_;
    if (std::find(allowable.begin(), allowable.end(), name) == allowable.end())
      full_names += name + " ";
  }

  if (!full_names.empty())
  {
    std::ostringstream msg;
    msg << "The criticality search will clear out all nuclides in material "
        << std::to_string(_material_id)
        << " and replace them with the naturally-abundant nuclides in hydrogen, oxygen, and boron. "
           "Any other nuclides which existed in the material will be deleted."
        << std::endl;
    msg << "\nThe material you provided contains nuclides which are not the natural isotopes of H, "
           "B, and O: "
        << full_names.substr(0, full_names.length() - 1) << ". These will be deleted from material "
        << std::to_string(_material_id) << " when the boron concentration is changed." << std::endl;
    msg << "\nFor general criticality searches based on material composition, please contact the "
           "Cardinal developer team.";
    mooseWarning(msg.str());
  }

  // Compute the molar mass of pure water
  _M_H2O = 0.0;
  for (const auto & h : _hydrogen_natural)
    _M_H2O += 2.0 * h.second * NuclearData::Nuclide::getAtomicMass(h.first);
  for (const auto & o : _oxygen_natural)
    _M_H2O += 1.0 * o.second * NuclearData::Nuclide::getAtomicMass(o.first);

  // Compute the molar mass of pure boron
  _M_B = 0.0;
  for (const auto & b : _boron_natural)
    _M_B += 1.0 * b.second * NuclearData::Nuclide::getAtomicMass(b.first);
}

void
BoratedWater::updateOpenMCModel(const Real & ppm)
{
  _console << "Searching for boron = " << ppm << " [ppm] ..." << std::endl;

  // A coupled thermal-fluid app may set the material density just before we enter
  // this routine; we will preserve that density which may be set and we interpret
  // it to be the SOLUTION density to be fully consistent with energy conservation.

  // Compute the number fractions of each element
  Real frac_H2O = (1 - ppm * 1e-6) / _M_H2O;
  Real frac_H = 2 * frac_H2O;
  Real frac_O = frac_H2O;
  Real frac_B = ppm * 1e-6 / _M_B;

  double rho;
  int err = openmc_material_get_density(_material_index, &rho);
  catchOpenMCError(err, "get density for material " + std::to_string(_material_id));

  std::vector<std::string> names;
  std::vector<double> densities;
  for (const auto & h : _hydrogen_natural)
  {
    names.push_back(h.first);
    densities.push_back(h.second * frac_H * rho * openmc::N_AVOGADRO);
  }
  for (const auto & o : _oxygen_natural)
  {
    names.push_back(o.first);
    densities.push_back(o.second * frac_O * rho * openmc::N_AVOGADRO);
  }
  for (const auto & b : _boron_natural)
  {
    names.push_back(b.first);
    densities.push_back(b.second * frac_B * rho * openmc::N_AVOGADRO);
  }

  openmc::model::materials[_material_index]->set_densities(names, densities /* atom/b-cm */);
}

void
BoratedWater::applyAbsentNuclides(const std::vector<std::string> & allowable)
{
  if (isParamValid("absent_nuclides"))
  {
    const auto & absent = getParam<std::vector<std::string>>("absent_nuclides");

    for (const auto & a : absent)
    {
      // only missing nuclides for H, O, B are meaningful
      if (std::find(allowable.begin(), allowable.end(), a) == allowable.end())
        paramWarning("absent_nuclides",
                     "Only absent isotopes of hydrogen, oxygen, or boron will be used to adjust "
                     "natural abundances. The entry '" +
                         a + "' will be unused.");

      // adjust the natural abundances if nuclides are missing from the cross section library
      // TODO: implement in a general fashion, this only works for O18 which is also the only
      // absent nuclide we expect in practice
      if (a == "O18")
      {
        // find which entry in _oxygen_natural has O16 and O18; this allows these isotopes
        // to appear in any order
        unsigned int idx16;
        unsigned int idx18;
        unsigned int idx = 0;
        for (const auto & o : _oxygen_natural)
        {
          if (o.first == "O16")
            idx16 = idx;
          if (o.first == "O18")
            idx18 = idx;
          idx++;
        }

        _oxygen_natural[idx16].second += _oxygen_natural[idx18].second;
        _oxygen_natural.erase(_oxygen_natural.begin() + idx18);
      }
      else
        paramError(
            "absent_nuclides",
            "Cardinal currently only assumes that O18 may be missing from your cross section "
            "library; please contact the Cardinal developer team to generalize this capability");
    }
  }
}
#endif
