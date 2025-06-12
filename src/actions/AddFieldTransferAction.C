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

#include "AddFieldTransferAction.h"
#include "NekRSProblem.h"
#include "FieldTransferBase.h"

registerMooseAction("CardinalApp", AddFieldTransferAction, "add_field_transfers");

InputParameters
AddFieldTransferAction::validParams()
{
  auto params = MooseObjectAction::validParams();
  params.addClassDescription("Adds a field transfer (mesh-based data) for coupling to NekRS");
  return params;
}

AddFieldTransferAction::AddFieldTransferAction(const InputParameters & parameters)
  : MooseObjectAction(parameters)
{
}

void
AddFieldTransferAction::act()
{
  if (_current_task == "add_field_transfers")
  {
    auto nek_problem = dynamic_cast<NekRSProblem *>(_problem.get());

    if (!nek_problem)
      mooseError("The [FieldTransfers] block can only be used with wrapped Nek cases! "
                 "You need to change the [Problem] block to 'NekRSProblem'.");

    if (_type == "NekFieldVariable" || _type == "NekVolumetricSource" ||
        _type == "NekBoundaryFlux" || _type == "NekMeshDeformation")
    {
      _moose_object_pars.set<NekRSProblem *>("_nek_problem") = nek_problem;
      auto transfer =
          nek_problem->addObject<FieldTransferBase>(_type, _name, _moose_object_pars, false)[0];
    }
  }
}
#endif
