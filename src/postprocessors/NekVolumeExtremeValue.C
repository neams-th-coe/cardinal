//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NekVolumeExtremeValue.h"
#include "NekInterface.h"

registerMooseObject("CardinalApp", NekVolumeExtremeValue);

defineLegacyParams(NekVolumeExtremeValue);

InputParameters
NekVolumeExtremeValue::validParams()
{
  InputParameters params = NekPostprocessor::validParams();
  params.addRequiredParam<MooseEnum>("field", getNekFieldEnum(), "Field to find the extreme value of");
  params.addParam<MooseEnum>("value_type", getOperationEnum(),
    "Whether to give the maximum or minimum extreme value; options: 'max' (default), 'min'");
  return params;
}

NekVolumeExtremeValue::NekVolumeExtremeValue(const InputParameters & parameters) :
  NekPostprocessor(parameters),
  _field(getParam<MooseEnum>("field").getEnum<field::NekFieldEnum>()),
  _type(getParam<MooseEnum>("value_type").getEnum<operation::OperationEnum>())
{
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
