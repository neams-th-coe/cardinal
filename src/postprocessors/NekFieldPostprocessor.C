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

#include "NekFieldPostprocessor.h"

defineLegacyParams(NekFieldPostprocessor);

InputParameters
NekFieldPostprocessor::validParams()
{
  InputParameters params = NekPostprocessor::validParams();
  params.addRequiredParam<MooseEnum>("field", getNekFieldEnum(), "Field to integrate; "
    "options: velocity_x, velocity_y, velocity_z, "
    "velocity_component, velocity, temperature, pressure, unity");
  params.addParam<Point>("velocity_direction",
    "Direction in which to evaluate velocity, for 'field = velocity_component'. For "
    "example, velocity_direction = '1 0 0' will get the x-component of velocity.");
  return params;
}

NekFieldPostprocessor::NekFieldPostprocessor(const InputParameters & parameters) :
  NekPostprocessor(parameters),
  _field(getParam<MooseEnum>("field").getEnum<field::NekFieldEnum>())
{
  checkValidField(_field);

  if (_field == field::velocity_component)
  {
    if (!isParamValid("velocity_direction"))
      mooseError("The 'velocity_direction' must be provided when using 'field = velocity_component'!");

    // get direction and convert to unit vector if not already a unit vector
    _velocity_direction = getParam<Point>("velocity_direction");

    Point zero(0.0, 0.0, 0.0);
    if (_velocity_direction.absolute_fuzzy_equals(zero))
      mooseError("The 'velocity_direction' vector cannot be the zero-vector!");

    _velocity_direction = _velocity_direction.unit();
  }
  else if (isParamValid("velocity_direction"))
      mooseWarning("The 'velocity_direction' parameter is unused unless 'field = velocity_component'!");
}
