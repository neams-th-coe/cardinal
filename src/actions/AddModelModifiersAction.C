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

#include "AddModelModifiersAction.h"
#include "OpenMCCellAverageProblem.h"
#include "ModelModifiersBase.h"

registerMooseAction("CardinalApp", AddModelModifiersAction, "add_model_modifiers");

InputParameters
AddModelModifiersAction::validParams()
{
  auto params = MooseObjectAction::validParams();
  params.addClassDescription("Adds a modifier that changes the OpenMC model prior to execution");
  return params;
}

AddModelModifiersAction::AddModelModifiersAction(const InputParameters & parameters)
  : MooseObjectAction(parameters)
{
}

void
AddModelModifiersAction::act()
{
  if (_current_task == "add_model_modifiers")
  {
    auto openmc_problem = dynamic_cast<OpenMCCellAverageProblem *>(_problem.get());

    if (!openmc_problem)
      mooseError("The [ModelModifiers] block can only be used with wrapped OpenMC cases! "
                 "You need to change the [Problem] block to 'OpenMCCellAverageProblem'.");

    openmc_problem->addObject<ModelModifiersBase>(_type, _name, _moose_object_pars, false);
  }
}
#endif
