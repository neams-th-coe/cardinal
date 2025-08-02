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

#include "AddCriticalitySearchAction.h"
#include "OpenMCCellAverageProblem.h"
#include "CriticalitySearchBase.h"

registerMooseAction("CardinalApp", AddCriticalitySearchAction, "add_criticality_search");

InputParameters
AddCriticalitySearchAction::validParams()
{
  auto params = MooseObjectAction::validParams();
  params.addClassDescription("Adds a criticality search for OpenMC");
  return params;
}

AddCriticalitySearchAction::AddCriticalitySearchAction(const InputParameters & parameters)
  : MooseObjectAction(parameters)
{
}

void
AddCriticalitySearchAction::act()
{
  if (_current_task == "add_criticality_search")
  {
    auto openmc_problem = dynamic_cast<OpenMCCellAverageProblem *>(_problem.get());

    if (!openmc_problem)
      mooseError("The [CriticalitySearch] block can only be used with wrapped OpenMC cases! "
                 "You need to change the [Problem] block to 'OpenMCCellAverageProblem'.");

    if (_type == "OpenMCMaterialDensity")
    {
      _moose_object_pars.set<OpenMCCellAverageProblem *>("_openmc_problem") = openmc_problem;
      auto search = openmc_problem->addObject<CriticalitySearchBase>(
          _type, _name, _moose_object_pars, false)[0];
    }
  }
}
#endif
