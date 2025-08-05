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

#ifdef ENABLE_NEK_COUPLING

#include "AddScalarTransferAction.h"
#include "NekRSProblem.h"
#include "ScalarTransferBase.h"

registerMooseAction("CardinalApp", AddScalarTransferAction, "add_scalar_transfers");

InputParameters
AddScalarTransferAction::validParams()
{
  auto params = MooseObjectAction::validParams();
  params.addClassDescription(
      "Adds a scalar transfer (a single number) for coupling NeKRS to MOOSE");
  return params;
}

AddScalarTransferAction::AddScalarTransferAction(const InputParameters & parameters)
  : MooseObjectAction(parameters)
{
}

void
AddScalarTransferAction::act()
{
  if (_current_task == "add_scalar_transfers")
  {
    auto nek_problem = dynamic_cast<NekRSProblem *>(_problem.get());

    if (!nek_problem)
      mooseError("The [ScalarTransfers] block can only be used with wrapped Nek cases! "
                 "You need to change the [Problem] block to 'NekRSProblem'.");

    _moose_object_pars.set<NekRSProblem *>("_nek_problem") = nek_problem;
    nek_problem->addObject<ScalarTransferBase>(_type, _name, _moose_object_pars, false)[0];
  }
}
#endif
