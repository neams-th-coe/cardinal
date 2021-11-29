#include "NekVolumeExtremeValue.h"
#include "NekInterface.h"

registerMooseObject("CardinalApp", NekVolumeExtremeValue);

InputParameters
NekVolumeExtremeValue::validParams()
{
  InputParameters params = NekFieldPostprocessor::validParams();
  params.addParam<MooseEnum>("value_type", getOperationEnum(),
    "Whether to give the maximum or minimum extreme value; options: 'max' (default), 'min'");
  params.addClassDescription("Compute the extreme value (max/min) of a field over the NekRS mesh");
  return params;
}

NekVolumeExtremeValue::NekVolumeExtremeValue(const InputParameters & parameters) :
  NekFieldPostprocessor(parameters),
  _type(getParam<MooseEnum>("value_type").getEnum<operation::OperationEnum>())
{
  if (_field == field::velocity_component)
    mooseError("Setting 'field = velocity_component' is not yet implemented!");
}

Real
NekVolumeExtremeValue::getValue()
{
  switch (_type)
  {
    case operation::max:
      return nekrs::volumeMaxValue(_field);
      break;
    case operation::min:
      return nekrs::volumeMinValue(_field);
      break;
    default:
      mooseError("Unhandled 'OperationEnum'!");
  }
}
