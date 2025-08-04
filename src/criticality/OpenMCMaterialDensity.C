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

#include "OpenMCMaterialDensity.h"
#include "UserErrorChecking.h"
#include "openmc/capi.h"

registerMooseObject("CardinalApp", OpenMCMaterialDensity);

InputParameters
OpenMCMaterialDensity::validParams()
{
  auto params = OpenMCMaterialSearch::validParams();
  params.addClassDescription("Searches for criticality using material density in units of kg/m3");
  return params;
}

OpenMCMaterialDensity::OpenMCMaterialDensity(const InputParameters & parameters)
  : OpenMCMaterialSearch(parameters)
{
  // a material in OpenMC must always have some nuclides in it or macroscopic data,
  // so we don't need to have any checks on whether the material is void

  // apply additional checks on the minimum and maximum values - both must be positive.
  // we don't need to check for negative 'maximum' because we already require maximum > minimum
  // and if we enforce non-negative minimum this will require maximum to also be non-negative.
  if (_minimum < 0.0)
    paramError("minimum", "The 'minimum' density (" + std::to_string(_minimum) + ") must be positive!");
}

void
OpenMCMaterialDensity::updateOpenMCModel(const Real & density)
{
  _console << "Searching for density = " << density << " [kg/m3] ..." << std::endl;

  const char * units = "g/cc";
  int err = openmc_material_set_density(
      _material_index, density * _openmc_problem->densityConversionFactor(), units);
  catchOpenMCError(err, "set material density to " + std::to_string(density));
}

#endif
