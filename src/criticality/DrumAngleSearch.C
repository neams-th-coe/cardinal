#ifdef ENABLE_OPENMC_COUPLING

#include "DrumAngleSearch.h"
#include "openmc/capi.h"
#include "UserObjectInterface.h"

// registerMooseObject("CardinalApp", DrumAngleSearch); // TODO

InputParameters
DrumAngleSearch::validParams()
{
  auto params = CriticalitySearchBase::validParams();
  params.addRequiredParam<UserObjectName>(
      "transform_name",
      "The OpenMCCellTransform UserObject that will control the criticality search");
  params.addRequiredParam<MooseEnum>(
      "rotation_axis", getRotationAxisEnum(), "Axis about which to rotate the cell. [x, y, z]");
  params.addClassDescription("Searches for criticality rotating a drum angle in degrees");

  return params;
}

DrumAngleSearch::DrumAngleSearch(const InputParameters & parameters)
  : CriticalitySearchBase(parameters),
    UserObjectInterface(this),
    _transform_name(getParam<UserObjectName>("transform_name")),
    _rotation_axis_char(getParam<MooseEnum>("rotation_axis"))
{
  // update the OpenMCCellTransform UserObject with the last iteration critical guess
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
DrumAngleSearch::updateOpenMCModel(const Real & angle)
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
  // _t->execute(); // update transform with new angle guess
}

#endif
