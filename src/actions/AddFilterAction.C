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
#include "AddFilterAction.h"

#include "OpenMCCellAverageProblem.h"

registerMooseAction("CardinalApp", AddFilterAction, "add_filters");

InputParameters
AddFilterAction::validParams()
{
  auto params = MooseObjectAction::validParams();
  params.addClassDescription("Adds filter(s) for use in simulations containing an "
                             "OpenMCCellAverageProblem.");

  return params;
}

AddFilterAction::AddFilterAction(const InputParameters & parameters) : MooseObjectAction(parameters)
{
}

void
AddFilterAction::act()
{
  if (_current_task == "add_filters")
  {
    auto openmc_problem = dynamic_cast<OpenMCCellAverageProblem *>(_problem.get());
    if (!openmc_problem)
      mooseError(
          "The simulation must use an OpenMCCellAverageProblem when using the filter system!");

    _moose_object_pars.set<OpenMCCellAverageProblem *>("_openmc_problem") = openmc_problem;
    openmc_problem->addFilter(_type, _name, _moose_object_pars);
  }
}
#endif
