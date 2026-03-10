#ifdef ENABLE_OPENMC_COUPLING

#include "RotationSearch.h"
#include "openmc/capi.h"
#include "UserObjectInterface.h"

InputParameters
RotationSearch::validParams()
{
  auto params = CriticalitySearchBase::validParams();
  params.addRequiredParam<UserObjectName>(
      "transform_name",
      "The OpenMCCellTransform UserObject that will control the criticality search");
  params.addRequiredParam<MooseEnum>(
      "rotation_axis", MooseEnum("x y z"), "Axis about which to rotate the cell's fill.");
  params.addClassDescription("Searches for criticality rotating a drum angle in degrees");

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
  if (_t->_transform_type != "rotation")
    paramError("transform_name",
               "You have attempted search for critical drum angle on the OpenMCCellTransform " +
                   _transform_name +
                   ", which does not modify a cell rotation."
                   "Please select a transform that rotates a control drum cell.");
}

void
RotationSearch::updateOpenMCModel(const Real & angle)
{
  _console << "Searching for drum angle = " << angle << units() << std::endl;

  switch (_rotation_axis_char)
  {
    case 'x':
      _t->setTransformPPValues(std::make_tuple(_t->_t0_pp_name, angle),
                               std::make_tuple(_t->_t1_pp_name, Real(0.0)),
                               std::make_tuple(_t->_t2_pp_name, Real(0.0)));
    case 'y':
      _t->setTransformPPValues(std::make_tuple(_t->_t0_pp_name, Real(0.0)),
                               std::make_tuple(_t->_t1_pp_name, angle),
                               std::make_tuple(_t->_t2_pp_name, Real(0.0)));
    case 'z':
      _t->setTransformPPValues(std::make_tuple(_t->_t0_pp_name, Real(0.0)),
                               std::make_tuple(_t->_t1_pp_name, Real(0.0)),
                               std::make_tuple(_t->_t2_pp_name, angle));
  }
}

#endif
