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

#include "OpenMCMaterialSearch.h"
#include "UserErrorChecking.h"
#include "openmc/capi.h"

InputParameters
OpenMCMaterialSearch::validParams()
{
  auto params = CriticalitySearchBase::validParams();
  params.addRequiredParam<int32_t>("material_id", "Material ID to modify");
  params.addClassDescription(
      "Base class for criticality searches using the properties of a material");
  return params;
}

OpenMCMaterialSearch::OpenMCMaterialSearch(const InputParameters & parameters)
  : CriticalitySearchBase(parameters), _material_id(getParam<int32_t>("material_id"))
{
  int err = openmc_get_material_index(_material_id, &_material_index);
  catchOpenMCError(err, "get index for material with ID " + std::to_string(_material_id));
}

#endif
