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

#include "NekUsrWrkBoundaryIntegral.h"

registerMooseObject("CardinalApp", NekUsrWrkBoundaryIntegral);

InputParameters
NekUsrWrkBoundaryIntegral::validParams()
{
  InputParameters params = NekSidePostprocessor::validParams();
  params.addRequiredParam<unsigned int>("usrwrk_slot", "Slot in nrs->usrwrk to integrate (zero-indexed)");
  params.addClassDescription("Compute integral of usrwrk over a boundary in the NekRS mesh");
  return params;
}

NekUsrWrkBoundaryIntegral::NekUsrWrkBoundaryIntegral(const InputParameters & parameters)
  : NekSidePostprocessor(parameters),
    _usrwrk_slot(getParam<unsigned int>("usrwrk_slot"))
{
  if (_usrwrk_slot >= _nek_problem->nUsrWrkSlots())
    mooseError("'usrwrk_slot' must be less than number of allocated usrwrk slots: ",
      _nek_problem->nUsrWrkSlots());
}

Real
NekUsrWrkBoundaryIntegral::getValue()
{
  auto integrals = nekrs::usrwrkSideIntegral(_usrwrk_slot, _boundary, _pp_mesh);
  return std::accumulate(integrals.begin(), integrals.end(), 0.0);
}

#endif
