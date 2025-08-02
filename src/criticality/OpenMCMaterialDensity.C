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
  auto params = CriticalitySearchBase::validParams();
  params.addRequiredParam<int32_t>("material_id", "Material ID for which to modify density");
  params.addClassDescription("Searches for criticality using material density; units for the density range are assumed to be kg/m3");
  return params;
}

OpenMCMaterialDensity::OpenMCMaterialDensity(const InputParameters & parameters)
  : CriticalitySearchBase(parameters),
    _material_id(getParam<int32_t>("material_id"))
{
  int err = openmc_get_material_index(_material_id, &_material_index);
  catchOpenMCError(err, "get index for material with ID " + std::to_string(_material_id));
}

void
OpenMCMaterialDensity::updateOpenMCModel(const Real & density)
{
  _console << "Searching for density = " << density << " [kg/m3] ..." << std::endl;

  const char * units = "g/cc";
  int err = openmc_material_set_density(_material_index, density * _openmc_problem->densityConversionFactor(), units);
  catchOpenMCError(err, "set material density to " + std::to_string(density));
}

#endif
