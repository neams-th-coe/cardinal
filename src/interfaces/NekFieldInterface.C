#include "NekFieldInterface.h"
#include "UserErrorChecking.h"
#include "GeometryUtils.h"

InputParameters
NekFieldInterface::validParams()
{
  InputParameters params = emptyInputParameters();
  params.addRequiredParam<MooseEnum>("field",
                                     getNekFieldEnum(),
                                     "Field to apply this object to");
  params.addParam<Point>(
      "velocity_direction",
      "Unit vector to dot with velocity, for 'field = velocity_component'. For "
      "example, velocity_direction = '1 0 0' will get the x-component of velocity.");
  return params;
}

NekFieldInterface::NekFieldInterface(const MooseObject * moose_object, const InputParameters & parameters)
  : _field(moose_object->getParam<MooseEnum>("field").getEnum<field::NekFieldEnum>())
{
  if (_field == field::velocity_component)
  {
    checkRequiredParam(moose_object->parameters(), "velocity_direction", "using 'field = velocity_component'");

    _velocity_direction =
        geom_utils::unitVector(moose_object->getParam<Point>("velocity_direction"), "velocity_direction");
  }
  else
    checkUnusedParam(moose_object->parameters(), "velocity_direction", "not using 'field = velocity_component'");
}
