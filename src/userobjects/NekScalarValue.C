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

#include "NekScalarValue.h"
#include "NekRSProblem.h"
#include "NekInterface.h"

registerMooseObject("CardinalApp", NekScalarValue);

InputParameters
NekScalarValue::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params.addParam<Real>("value", 0.0, "Scalar value to pass into NekRS");
  params.addParam<Real>(
      "scaling",
      1.0,
      "Multiplier on 'value', typically used to convert from dimensional form into NekRS's "
      "non-dimensional form, if using a non-dimensional NekRS solve.");
  params.addRequiredParam<unsigned int>("usrwrk_slot", "Slot in nrs->usrwrk into which to write the value");
  params.declareControllable("value");
  params.addClassDescription("Writes a scalar value from MOOSE into NekRS's scratch space");
  return params;
}

NekScalarValue::NekScalarValue(const InputParameters & parameters)
  : GeneralUserObject(parameters),
    _value(getParam<Real>("value")),
    _scaling(getParam<Real>("scaling")),
    _usrwrk_slot(getParam<unsigned int>("usrwrk_slot"))
{
  const NekRSProblem * nek_problem = dynamic_cast<const NekRSProblem *>(&_fe_problem);
  if (!nek_problem)
  {
    std::string extra_help = _fe_problem.type() == "FEProblem" ? " (the default)" : "";
    mooseError("This user object can only be used with wrapped Nek cases!\n"
               "You need to change the problem type from '" +
               _fe_problem.type() + "'" + extra_help +
               " to 'NekRSProblem'.");
  }

  auto field_usrwrk_map = nek_problem->fieldUsrwrkMap();

  // check that we're not writing into space that's used for field transfers
  if (field_usrwrk_map.find(_usrwrk_slot) != field_usrwrk_map.end())
  {
    std::string unavailable_slots = "";
    for (const auto & f : field_usrwrk_map)
      unavailable_slots += Moose::stringify(f) + " ";

    paramError("usrwrk_slot", "The usrwrk slot " + Moose::stringify(_usrwrk_slot) + " is already used by the FieldTransfers for writing field data into NekRS. You cannot set 'usrwrk_slot' to any of: " + unavailable_slots);
  }

  if (_usrwrk_slot >= nek_problem->nUsrWrkSlots())
    paramError("usrwrk_slot", "This parameter cannot exceed the available pre-allocated slots (",
      Moose::stringify(nek_problem->nUsrWrkSlots()), "). Please either change this parameter, "
      "or increase 'n_usrwrk_slots' in the [Problem] block.");
}

void
NekScalarValue::setValue()
{
  nrs_t * nrs = (nrs_t *) nekrs::nrsPtr();
  nrs->usrwrk[_usrwrk_slot * nekrs::fieldOffset() + _counter] = _value * _scaling;
}

Real
NekScalarValue::getValue() const
{
  nrs_t * nrs = (nrs_t *) nekrs::nrsPtr();
  return nrs->usrwrk[_usrwrk_slot * nekrs::fieldOffset() + _counter];
}

#endif
