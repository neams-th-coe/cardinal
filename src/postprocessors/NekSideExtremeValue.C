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

#include "NekSideExtremeValue.h"

registerMooseObject("CardinalApp", NekSideExtremeValue);

InputParameters
NekSideExtremeValue::validParams()
{
  InputParameters params = NekSideFieldPostprocessor::validParams();
  params.addParam<MooseEnum>(
      "value_type",
      getOperationEnum(),
      "Whether to give the maximum or minimum extreme value");
  params.addClassDescription(
      "Compute the extreme value (max/min) of a field over a boundary of the NekRS mesh");
  return params;
}

NekSideExtremeValue::NekSideExtremeValue(const InputParameters & parameters)
  : NekSideFieldPostprocessor(parameters),
    _type(getParam<MooseEnum>("value_type").getEnum<operation::OperationEnum>())
{
  if (_field == field::velocity_component)
    mooseError("Setting 'field = velocity_component' is not yet implemented!");
}

Real
NekSideExtremeValue::getValue()
{
  switch (_type)
  {
    case operation::max:
      return nekrs::sideExtremeValue(_boundary, _field, _pp_mesh, true /* max */);
      break;
    case operation::min:
      return nekrs::sideExtremeValue(_boundary, _field, _pp_mesh, false /* min */);
      break;
    default:
      mooseError("Unhandled 'OperationEnum'!");
  }
}

#endif
