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

registerMooseObject("CardinalApp", NekScalarValue);

InputParameters
NekScalarValue::validParams()
{
  auto params = ScalarTransferBase::validParams();
  params.addParam<Real>("value", 0.0, "Scalar value to pass into NekRS");
  params.addParam<PostprocessorName>(
      "output_postprocessor", "Name of the postprocessor to output the value sent into NekRS");
  params.declareControllable("value");

  params.addClassDescription("Transfers a scalar value into NekRS");
  params.registerBase("ScalarTransfer");
  params.registerSystemAttributeName("ScalarTransfer");
  return params;
}

NekScalarValue::NekScalarValue(const InputParameters & parameters)
  : ScalarTransferBase(parameters),
    _value(getParam<Real>("value")),
    _postprocessor(isParamValid("output_postprocessor")
                       ? &getParam<PostprocessorName>("output_postprocessor")
                       : nullptr)
{
}

void
NekScalarValue::sendDataToNek()
{
  Real value_to_set = _value * _scaling;
  _console << "Sending scalar value (" << Moose::stringify(value_to_set) << ") to NekRS..."
           << std::endl;

  auto nrs = nekrs::nrsPtr();
  auto usrwrk = nekrs::host_wrk();
  usrwrk[_usrwrk_slot * nekrs::fieldOffset() + _offset] = value_to_set;

  if (_postprocessor)
    _nek_problem.setPostprocessorValueByName(*_postprocessor, value_to_set);
}

#endif
