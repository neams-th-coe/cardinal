#ifdef ENABLE_OPENMC_COUPLING

#include "RotationSearch.h"
#include "openmc/capi.h"

registerMooseObject("CardinalApp", RotationSearch);

InputParameters
RotationSearch::validParams()
{
  auto params = CriticalitySearchBase::validParams();
  params += OpenMCCellTransformBase::validParams();
  params.addRequiredParam<MooseEnum>(
      "rotation_axis", MooseEnum("x=0 y=1 z=2"), "Axis about which to rotate the cell's fill.");
  params.addClassDescription("Searches for criticality by modifying cell rotation(s) in degrees");

  return params;
}

RotationSearch::RotationSearch(const InputParameters & parameters)
  : CriticalitySearchBase(parameters),
    OpenMCCellTransformBase(static_cast<MooseObject &>(*this)),
    _rotation_axis_idx(int(getParam<MooseEnum>("rotation_axis")))
{
  // apply additional checks on the minimum and maximum to ensure one period of rotation in search
  if (_minimum < 0)
    paramError("minimum",
               "The 'minimum' specified rotation (" + std::to_string(_minimum) +
                   ") must be positive!");
  if (_maximum >= 360)
    paramError("The 'maximum' specified rotation (" + std::to_string(_maximum) +
               ") must be less than 360 degrees");
}

void
RotationSearch::updateOpenMCModel(const Real & angle)
{
  _console << "OpenMC will run with next guess for rotation = " << angle << units() << std::endl;

  // make a vectorized version of the rotation angle with 0 (default Point value) for the
  // non-rotating axes
  Point angles;
  // the enum default indices correspond to which vector component is non zero
  angles(_rotation_axis_idx) = angle;

  // rotation transformation
  auto transform_type_enum = OpenMCCellTransformBase::transform_type;
  transform_type_enum = "rotation";

  // do the transform
  transform(transform_type_enum, angles);
}

#endif
