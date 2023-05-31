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

#include "NekVolumeExtremeValue.h"
#include "NekInterface.h"

registerMooseObject("CardinalApp", NekVolumeExtremeValue);

InputParameters
NekVolumeExtremeValue::validParams()
{
  InputParameters params = NekFieldPostprocessor::validParams();
  params.addParam<MooseEnum>(
      "value_type",
      getOperationEnum(),
      "Whether to give the maximum or minimum extreme value");
  params.addClassDescription("Compute the extreme value (max/min) of a field over the NekRS mesh");
  return params;
}

NekVolumeExtremeValue::NekVolumeExtremeValue(const InputParameters & parameters)
  : NekFieldPostprocessor(parameters),
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
      return nekrs::volumeExtremeValue(_field, _pp_mesh, true /* max */);
      break;
    case operation::min:
      return nekrs::volumeExtremeValue(_field, _pp_mesh, false /* min */);
      break;
    default:
      mooseError("Unhandled 'OperationEnum'!");
  }
}

#endif
