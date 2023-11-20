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

#include "OpenMCNuclideDensitiesControl.h"

#include "CardinalAppTypes.h"
#include "OpenMCNuclideDensities.h"

registerMooseObject("CardinalApp", OpenMCNuclideDensitiesControl);

InputParameters
OpenMCNuclideDensitiesControl::validParams()
{
  InputParameters params = Control::validParams();
  params.addClassDescription("Controls the densities in a OpeNMCNuclideDensities object.");
  params.addRequiredParam<UserObjectName>("user_object",
                                          "The name of the associated OpenMCNuclideDensities");

  // each index in the vectors represents one time step
  params.addRequiredParam<std::vector<std::vector<std::string>>>(
      "names", "Names of the nuclides to modifiy densities for each timestep");
  params.addRequiredParam<std::vector<std::vector<Real>>>(
      "densities", "Nuclide densities (atom/b/cm) to set for each timestep");

  auto & exec_enum = params.set<ExecFlagEnum>("execute_on", true);
  exec_enum.addAvailableFlags(EXEC_SEND_OPENMC_DENSITIES);
  exec_enum = {EXEC_SEND_OPENMC_DENSITIES};
  params.suppressParameter<ExecFlagEnum>("execute_on");

  return params;
}

OpenMCNuclideDensitiesControl::OpenMCNuclideDensitiesControl(const InputParameters & parameters)
  : Control(parameters),
    _controllable_prefix("UserObjects/" + getParam<UserObjectName>("user_object") + "/"),
    _names(getParam<std::vector<std::vector<std::string>>>("names")),
    _densities(getParam<std::vector<std::vector<Real>>>("densities")),
    _current_execution(0)
{
  // This will make sure that the associated name is actually a OpenMCNuclideDensities
  if (!hasUserObject<OpenMCNuclideDensities>("user_object"))
    paramError("user_object",
               "The given UserObject does not exist or it is not a OpenMCNuclideDensities object");

  // do not need any error checking here, because we perform it in OpenMCNuclideDensities
}

void
OpenMCNuclideDensitiesControl::execute()
{
  if (_current_execution >= _names.size())
  {
    mooseWarning("Skipping nuclide control as data was not provided for this execution");
    return;
  }

  setControllableValueByName<std::vector<std::string>>(_controllable_prefix + "names",
                                                       _names[_current_execution]);
  setControllableValueByName<std::vector<Real>>(_controllable_prefix + "densities",
                                                _densities[_current_execution]);

  ++_current_execution;
}

#endif
