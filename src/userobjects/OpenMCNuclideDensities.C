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

#include "OpenMCNuclideDensities.h"
#include "UserErrorChecking.h"
#include "openmc/material.h"

registerMooseObject("CardinalApp", OpenMCNuclideDensities);

InputParameters
OpenMCNuclideDensities::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params += OpenMCBase::validParams();
  params.addRequiredParam<int32_t>("material_id", "ID of material to change nuclide densities");
  params.addRequiredParam<std::vector<std::string>>("names",
                                                    "Names of the nuclides to modify densities");
  params.addRequiredParam<std::vector<double>>("densities", "Nuclide densities (atom/b/cm) to set");
  params.declareControllable("names");
  params.declareControllable("densities");
  params.addClassDescription("Updates nuclide densities in an OpenMC material");
  return params;
}

OpenMCNuclideDensities::OpenMCNuclideDensities(const InputParameters & parameters)
  : GeneralUserObject(parameters),
    OpenMCBase(this, parameters),
    _material_id(getParam<int32_t>("material_id")),
    _names(getParam<std::vector<std::string>>("names")),
    _densities(getParam<std::vector<double>>("densities"))
{
  catchOpenMCError(openmc_get_material_index(_material_id, &_material_index),
                   "get the material index for material with ID " +
                   std::to_string(_material_id));
}

void
OpenMCNuclideDensities::setValue()
{
  if (_names.size() == 0)
    paramError("names", "'names' cannot be of length zero!");

  if (_names.size() != _densities.size())
    mooseError("'names' and 'densities' must be the same length!");

  try
  {
    openmc::model::materials[_material_index]->set_densities(_names, _densities);
  }
  catch (const std::exception & e)
  {
    mooseError("In attempting to set nuclide densities in the '" + name() +
               "' UserObject, OpenMC reported:\n\n" + e.what());
  }
}

#endif
