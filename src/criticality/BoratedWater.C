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

#include "BoratedWater.h"
#include "UserErrorChecking.h"
#include "openmc/capi.h"
#include "openmc/nuclide.h"

registerMooseObject("CardinalApp", BoratedWater);

InputParameters
BoratedWater::validParams()
{
  auto params = OpenMCMaterialSearch::validParams();
  params.addClassDescription("Searches for criticality using natural boron ppm in water in units of weight ppm");
  return params;
}

BoratedWater::BoratedWater(const InputParameters & parameters)
  : OpenMCMaterialSearch(parameters)
{
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

  // TODO: change to account for isotopes
  std::vector<std::string> allowable = {"H", "O", "B"};
  std::string full_names = "";
  bool found_others = false;
  for (int i = 0; i < n; ++i)
  {
    std::string name = openmc::data::nuclides[i]->name_;
    name.erase(std::remove_if(name.begin(), name.end(), [](char c) {
        return std::isdigit(c);
    }), name.end());

    if (std::find(allowable.begin(), allowable.end(), name) == allowable.end())
    {
      found_others = true;
      full_names += openmc::data::nuclides[i]->name_ + " ";
    }
  }

  if (!full_names.empty())
    mooseWarning("The criticality search will clear out all nuclides in material " + std::to_string(_material_id) + " and replace them with the naturally-abundant nuclides in hydrogen, oxygen, and boron. Any other nuclides which existed in the material, such as corrosion products or chemical species for redox control will be deleted! The material you provided contains non-H, O, or B nuclides: " + full_names.substr(0, full_names.length() - 1) + ". These will be deleted from material " + std::to_string(_material_id) + " when the boron concentration is changed.");
}

void
BoratedWater::updateOpenMCModel(const Real & ppm)
{
  _console << "Searching for boron = " << ppm << " [ppm] ..." << std::endl;
}

#endif
