#include "NekSideFieldPostprocessor.h"

defineLegacyParams(NekSideFieldPostprocessor);

InputParameters
NekSideFieldPostprocessor::validParams()
{
  InputParameters params = NekSidePostprocessor::validParams();
  params.addRequiredParam<MooseEnum>("field", getNekFieldEnum(), "Field to integrate;"
    "options: velocity_x, velocity_y, velocity_z, "
    "velocity_component, velocity, temperature, pressure, unity");
  params.addParam<Point>("velocity_direction",
    "Direction in which to evaluate velocity, for 'field = velocity_component'. For "
    "example, velocity_direction = '1 0 0' will get the x-component of velocity.");
  return params;
}

NekSideFieldPostprocessor::NekSideFieldPostprocessor(const InputParameters & parameters) :
  NekSidePostprocessor(parameters),
  _field(getParam<MooseEnum>("field").getEnum<field::NekFieldEnum>())
{
  checkValidField(_field);

  if (_field == field::velocity_component)
  {
    if (!isParamValid("velocity_direction"))
      mooseError("The 'velocity_direction' must be provided when using 'field = velocity_component'!");

    // get direction and convert to unit vector if not already a unit vector
    _velocity_direction = getParam<Point>("velocity_direction").unit();
  }
  else if (isParamValid("velocity_direction"))
      mooseWarning("The 'velocity_direction' parameter is unused unless 'field = velocity_component'!");
}
