#ifdef ENABLE_OPENMC_COUPLING

#include "RotationSearch.h"
#include "openmc/capi.h"

registerMooseObject("CardinalApp", RotationSearch);

InputParameters
RotationSearch::validParams()
{
  auto params = CriticalitySearchBase::validParams();
  params.addRequiredParam<UserObjectName>(
      "transform_name",
      "The OpenMCCellTransform UserObject that will control the criticality search");
  params.addRequiredParam<MooseEnum>(
      "rotation_axis", MooseEnum("x y z"), "Axis about which to rotate the cell's fill.");
  params.addClassDescription("Searches for criticality by modifying cell rotation(s) in degrees");

  return params;
}

RotationSearch::RotationSearch(const InputParameters & parameters)
  : CriticalitySearchBase(parameters),
    UserObjectInterface(this),
    _transform_name(getParam<UserObjectName>("transform_name")),
    _rotation_axis_char(getParam<MooseEnum>("rotation_axis"))
{
  // update the OpenMCCellTransform UserObject with the last iteration critical guess
  try
  {
    &getUserObjectByName<OpenMCCellTransform>(_transform_name);
  }
  catch (const std::exception & e)
  {
    std::string s = e.what();
    paramError(
        "transform_name",
        "In attempting to get the OpenMCCellTransform UserObject, the following error occurred: " +
            s +
            "\nThis likely means that the name provided either is incorrect or is not an "
            "OpenMCCellTransform UserObject.");
  }
  _t =
      const_cast<OpenMCCellTransform *>(&getUserObjectByName<OpenMCCellTransform>(_transform_name));
  // check that the specified transform is a rotational transform
  if (_t->getTransformType() != "rotation")
    paramError("transform_name",
               "You have attempted search for critical drum angle on the OpenMCCellTransform " +
                   _transform_name +
                   ", which does not modify a cell rotation."
                   "Please select a transform that rotates a control drum cell.");
  // confirm that rotation specified is valid for a criticality search
  _t->checkTransformIsValidRotationForCriticalitySearch();
}

void
RotationSearch::updateOpenMCModel(const Real & angle)
{
  _console << "OpenMC will run with next guess for rotation = " << angle << units() << std::endl;

  // make a vectorized version of the rotation angle with 0 for the non-rotating axes
  std::vector<Real> angles = {Real(0.0), Real(0.0), Real(0.0)};
  angles[int(_rotation_axis_char)] =
      angle; // the enum default indices correspond to which vector component is non zero

  // set the transform values using the angle vector
  _t->setTransformPPValues(angles);
  _t->execute();
}

#endif
