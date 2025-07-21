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

#include "NekRSProblem.h"
#include "NekRSMesh.h"
#include "NekTransferBase.h"

InputParameters
NekTransferBase::validParams()
{
  auto params = MooseObject::validParams();
  MooseEnum direction("to_nek from_nek");
  params.addRequiredParam<MooseEnum>("direction", direction, "Direction in which to send data");
  params.addClassDescription("Base class for passing data between NekRS and MOOSE");
  params.addPrivateParam<NekRSProblem *>("_nek_problem");
  return params;
}

NekTransferBase::NekTransferBase(const InputParameters & parameters)
  : MooseObject(parameters),
    PostprocessorInterface(this),
    _nek_problem(*getParam<NekRSProblem *>("_nek_problem")),
    _direction(getParam<MooseEnum>("direction"))
{
  // we do not need to check for other mesh types because we already enforce
  // the usage of NekRSMesh by requiring the use of a NekRS problem
  // in the AddNekTransferAction
  _nek_mesh = dynamic_cast<NekRSMesh *>(&(getMooseApp().feProblem().mesh()));
}

void
NekTransferBase::checkAllocatedUsrwrkSlot(const unsigned int & u) const
{
  if (u >= _nek_problem.nUsrWrkSlots())
  {
    std::string s = "Cannot write into usrwrk slot " + Moose::stringify(u) + " because only " + Moose::stringify(_nek_problem.nUsrWrkSlots()) + " have been allocated with 'n_usrwrk_slots'.";

    // can only give the hint about max slot if any slots have been allocated, otherwise we overflow
    // into max(int)
    if (_nek_problem.nUsrWrkSlots() > 0)
      s += " Slots are zero-indexed, so the maximum acceptable value in 'usrwrk_slot' is " +
           Moose::stringify(_nek_problem.nUsrWrkSlots() - 1) + ".";
    paramError("usrwrk_slot", s + " You must increase 'n_usrwrk_slots' in the [Problem] block.");
  }
}

void
NekTransferBase::addExternalPostprocessor(const std::string name, const Real initial)
{
  auto pp_params = _factory.getValidParams("Receiver");
  pp_params.set<Real>("default") = initial;

  // we do not need to check for duplicate names, because MOOSE already handles
  // this error checking
  _nek_problem.addPostprocessor("Receiver", name, pp_params);
}

#endif
