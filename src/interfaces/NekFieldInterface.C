#include "NekFieldInterface.h"
#include "UserErrorChecking.h"
#include "GeometryUtils.h"

InputParameters
NekFieldInterface::validParams()
{
  InputParameters params = emptyInputParameters();
  params.addRequiredParam<MooseEnum>("field", getNekFieldEnum(), "Field to apply this object to");
  params.addParam<Point>("velocity_direction",
                         "Unit vector to dot with velocity, for 'field = velocity_component'. For "
                         "example, velocity_direction = '1 1 0' will get the velocity dotted with "
                         "(1/sqrt(2), 1/sqrt(2), 0).");
  params.addParam<MooseEnum>(
      "velocity_component",
      getBinnedVelocityComponentEnum(),
      "Direction in which to evaluate velocity when 'field = velocity_component.' "
      "Options: user (you then need to specify a direction with 'velocity_direction'); normal");
  return params;
}

NekFieldInterface::NekFieldInterface(const MooseObject * moose_object,
                                     const InputParameters & parameters,
                                     const bool allow_normal_velocity)
  : _field(moose_object->getParam<MooseEnum>("field").getEnum<field::NekFieldEnum>()),
    _velocity_component(moose_object->getParam<MooseEnum>("velocity_component")
                            .getEnum<component::BinnedVelocityComponentEnum>())
{
  if (_field == field::velocity_component)
  {
    switch (_velocity_component)
    {
      case component::user:
        checkRequiredParam(parameters, "velocity_direction", "using 'velocity_component = user'");

        _velocity_direction = geom_utils::unitVector(
            moose_object->getParam<Point>("velocity_direction"), "velocity_direction");

        break;
      case component::normal:
        if (!allow_normal_velocity)
          mooseError("This object does not support 'velocity_component = normal'! Please contact "
                     "the Cardinal development team if this is hindering your use case.");

        checkUnusedParam(parameters, "velocity_direction", "using 'velocity_component = normal'");
        break;
      default:
        mooseError("Unhandled BinnedVelocityComponentEnum!");
    }
  }
  else
  {
    checkUnusedParam(parameters, "velocity_direction", "not using 'field = velocity_component'");
    checkUnusedParam(parameters, "velocity_component", "not using 'field = velocity_component'");
  }
}
