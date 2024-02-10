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

#include "NekSideFieldPostprocessor.h"
#include "UserErrorChecking.h"

InputParameters
NekSideFieldPostprocessor::validParams()
{
  InputParameters params = NekSidePostprocessor::validParams();
  params.addRequiredParam<MooseEnum>("field",
                                     getNekFieldEnum(),
                                     "Field to integrate");
  params.addParam<Point>(
      "velocity_direction",
      "Direction in which to evaluate velocity, for 'field = velocity_component'. For "
      "example, velocity_direction = '1 0 0' will get the x-component of velocity.");
  return params;
}

NekSideFieldPostprocessor::NekSideFieldPostprocessor(const InputParameters & parameters)
  : NekSidePostprocessor(parameters),
    _field(getParam<MooseEnum>("field").getEnum<field::NekFieldEnum>())
{
  if (_field == field::velocity_component)
  {
    checkRequiredParam(parameters, "velocity_direction", "using 'field = velocity_component'");
    _velocity_direction =
        geom_utils::unitVector(getParam<Point>("velocity_direction"), "velocity_direction");
  }
  else
    checkUnusedParam(parameters, "velocity_direction", "not using 'field = velocity_component'");
}

#endif
