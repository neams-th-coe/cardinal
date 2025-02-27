#include "NekFieldInterface.h"
#include "UserErrorChecking.h"
#include "GeometryUtils.h"
#include "NekRSProblemBase.h"

InputParameters
NekFieldInterface::validParams()
{
  InputParameters params = emptyInputParameters();
  params.addRequiredParam<MooseEnum>("field", getNekFieldEnum(), "Field to apply this object to");
  params.addParam<FunctionName>(
      "function",
      "Function to shift the field by, when applying this object. For example, if 'field = "
      "temperature', using this parameter will apply this object to the new combined quantity "
      "'temperature - f', where 'f' is this shifting function.");
  params.addParam<Point>(
      "velocity_direction",
      "Unit vector to dot with velocity, for 'field = velocity_component'. For "
      "example, velocity_direction = '1 0 0' will get the x-component of velocity.");
  return params;
}

NekFieldInterface::NekFieldInterface(const MooseObject * moose_object,
                                     const InputParameters & parameters)
  : FunctionInterface(moose_object),
    _field(moose_object->getParam<MooseEnum>("field").getEnum<field::NekFieldEnum>()),
    _function(moose_object->isParamValid("function") ? &this->getFunction("function") : nullptr)
{
  if (_field == field::velocity_component)
  {
    checkRequiredParam(
        moose_object->parameters(), "velocity_direction", "using 'field = velocity_component'");

    _velocity_direction = geom_utils::unitVector(
        moose_object->getParam<Point>("velocity_direction"), "velocity_direction");
  }
  else
    checkUnusedParam(
        moose_object->parameters(), "velocity_direction", "not using 'field = velocity_component'");
}

Real
NekFieldInterface::evaluateShiftFunction(const Real & time, const Point & point) const
{
  // the input functions are dimensional quantities; first, need to transform
  // them into non-dimensional form before NekRS evaluates them
  auto t = time / nekrs::referenceTime();
  auto p = point / nekrs::referenceLength();

  // if there is a shifting function, evaluate that function
  return _function ? _function->value(t, p) : 0.0;
}
