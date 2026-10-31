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

#include "NekUsrWrkSTLIntegral.h"

registerMooseObject("CardinalApp", NekUsrWrkSTLIntegral);

InputParameters
NekUsrWrkSTLIntegral::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  params.addRequiredParam<unsigned int>(
      "usrwrk_slot", "Slot in platform->app->bc->o_usrwrk to integrate (zero-indexed)");
  params.addClassDescription("Integral of usrwrk over an STL file");
  return params;
}

NekUsrWrkSTLIntegral::NekUsrWrkSTLIntegral(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    NekBase(this, parameters),
    _usrwrk_slot(getParam<unsigned int>("usrwrk_slot"))
{
  if (_usrwrk_slot >= _nek_problem->nUsrWrkSlots())
    mooseError("'usrwrk_slot' must be less than number of allocated usrwrk slots: ",
      _nek_problem->nUsrWrkSlots());

  if (!_nek_mesh->hasSTLFile())
    mooseError("This object can only be used when an 'stl' file is provided to NekRSMesh");
}

Real
NekUsrWrkSTLIntegral::getValue() const
{
  return nekrs::usrwrkSTLIntegral(_usrwrk_slot * nekrs::fieldOffset(), _nek_mesh->STLVertices(), _nek_mesh->numSTLElems());
}

#endif
