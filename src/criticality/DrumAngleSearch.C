#ifdef ENABLE_OPENMC_COUPLING

#include "DrumAngleSearch.h"
#include "openmc/capi.h"

registerMooseObject("CardinalApp", DrumAngleSearch); // TODO

InputParameters
DrumAngleSearch::validParams()
{
  auto params = CriticalitySearchBase::validParams();
  params.addRequiredParam<std::string>(
      "transform_name",
      "The OpenMCCellTransform UserObject that will control the criticality search");
  params.addRequiredParam<MooseEnum>(
      "rotation_axis", getRotationAxisEnum(), "Axis about which to rotate the cell. [x, y, z]");
  params.addClassDescription("Searches for criticality rotating a drum angle in degrees");

  return params;
}

DrumAngleSearch::DrumAngleSearch(const InputParameters & parameters)
  : CriticalitySearchBase(parameters),
    _rotation_axis_char(getParam<MooseEnum>("rotation_axis")),
    _transform_name(getParam<std::string>("transform_name")),
{
  // update the OpenMCCellTransform UserObject with the last iteration critical guess
  const auto & _t = getUserObjectByName<OpenMCCellTransform>(_transform_name);
  // const auto _t = &getUserObjectByName<OpenMCCellTransform>(_transform_name);

  // check that the specified transform is a rotational transform
  if (_t._transform_type != "rotation")
    paramError("transform_name",
               "You have attempted search for critical drum angle on the OpenMCCellTransform " +
                   std::to_string(_transform_name) +
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
      _t->_t0_pp = angle;
      _t->_t1_pp = 0.0;
      _t->_t2_pp = 0.0;
    case 'y':
      _t->_t0_pp = 0.0;
      _t->_t1_pp = angle;
      _t->_t2_pp = 0.0;
    case 'z':
      _t->_t0_pp = 0.0;
      _t->_t1_pp = 0.0;
      _t->_t2_pp = angle;
  }
  _t->execute(); // update transform with new critical angle
}

#endif
