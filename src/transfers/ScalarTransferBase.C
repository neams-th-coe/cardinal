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

#include "ScalarTransferBase.h"

std::map<unsigned int, unsigned int> ScalarTransferBase::_counter;

InputParameters
ScalarTransferBase::validParams()
{
  auto params = NekTransferBase::validParams();
  params.addParam<unsigned int>(
      "usrwrk_slot",
      "When 'direction = to_nek', the slot in the usrwrk array to write the incoming data");
  params.addParam<Real>("scaling", 1.0, "Multiplier on the value passed into NekRS");
  params.addClassDescription(
      "Base class for defining input parameters for passing single values (scalars) into NekRS");
  return params;
}

ScalarTransferBase::ScalarTransferBase(const InputParameters & parameters)
  : NekTransferBase(parameters), _scaling(getParam<Real>("scaling"))
{
  if (_direction == "to_nek")
  {
    checkRequiredParam(parameters, "usrwrk_slot", "writing data 'to_nek'");
    _usrwrk_slot = getParam<unsigned int>("usrwrk_slot");

    // slot should not be greater than the amount allocated
    checkAllocatedUsrwrkSlot(_usrwrk_slot);

    // check that we're not writing into space that's used for field transfers
    auto field_usrwrk_map = FieldTransferBase::usrwrkMap();
    if (field_usrwrk_map.find(_usrwrk_slot) != field_usrwrk_map.end())
    {
      std::string unavailable_slots = "";
      for (const auto & f : field_usrwrk_map)
        unavailable_slots += Moose::stringify(f.first) + " ";

      paramError("usrwrk_slot",
                 "The usrwrk slot " + Moose::stringify(_usrwrk_slot) +
                     " is already used by the FieldTransfers for writing field data into NekRS. "
                     "You cannot set 'usrwrk_slot' to any of: " +
                     unavailable_slots);
    }

    // we do allow duplicates now within the scalar transfers; we simply shift
    // each by an offset; first, find what the previous filled offset was for this
    // slot, then save that value for usage
    if (_counter.find(_usrwrk_slot) == _counter.end())
      _counter[_usrwrk_slot] = 0;
    else
      _counter[_usrwrk_slot] += 1;

    _offset = _counter[_usrwrk_slot];
  }

  if (_direction == "from_nek")
    paramError("direction",
               "Reading scalar values from NekRS can be performed using the Postprocessing system. "
               "Alternatively, if you want to pull out individual scalars from nrs->usrwrk, "
               "contact the Cardinal developer team to request this feature.");
}

#endif
