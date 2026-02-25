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
    PostprocessorInterface(this),
    _transform_name(getParam<UserObjectName>("transform_name")),
    _rotation_axis_idx(int(getParam<MooseEnum>("rotation_axis")))
{
  _t = const_cast<OpenMCCellTransform *>(&getUserObject<OpenMCCellTransform>("transform_name"));
  // check that the specified transform is a rotational transform
  if (_t->getTransformType() != "rotation")
    paramError(
        "transform_name",
        "You have attempted to search for critical rotation angle on the OpenMCCellTransform " +
            _transform_name +
            ", which does not modify a cell rotation."
            "Please select a transform that rotates a cell.");

  // confirm that vector_value member of the specified OpenMCCellTransform is valid for a
  // RotationSearch
  checkValidVectorValueForRotationSearch();
}

void
RotationSearch::checkValidVectorValueForRotationSearch()
{
  const auto vv = _t->getVectorValue();

  // check vector_value to make sure that the rotation axis position is the only non-zero entry
  for (int idx = 0; idx < vv.size(); idx++)
  {
    // if a numeric value was specified as an entry in vector_value it will be recognizable as a
    // float to MooseUtils
    if (MooseUtils::isFloat(vv[idx]))
    {
      if (idx == _rotation_axis_idx)
      {
        _t->paramError("vector_value",
                       "The entry corresponding to the specified rotation axis is not a "
                       "Postprocessor, which is required for a RotationSearch.");
      } // implied else
      if (std::abs(std::stof(vv[idx])) > 1e-6)
      {
        _t->paramError("vector_value",
                       "An entry that is not the rotation axis returned a non-zero value, which is "
                       "not allowed.");
      }
    }
    else
    {
      // entering this block confirms that an entry is a PostprocessorName, of which there can only
      // be one and it must be the coordinate that aligns with the _rotation_axis_char
      if (idx != _rotation_axis_idx)
      {
        _t->paramError("vector_value",
                       "Only one component of `vector_value` can be non-zero for a RotationSearch "
                       "and it must be a PostprocessorName.");
      }
    }
  }
}

void
RotationSearch::updateOpenMCModel(const Real & angle)
{
  _console << "OpenMC will run with next guess for rotation = " << angle << units() << std::endl;

  // make a vectorized version of the rotation angle with 0 for the non-rotating axes
  std::vector<Real> angles = {Real(0.0), Real(0.0), Real(0.0)};
  angles[_rotation_axis_idx] =
      angle; // the enum default indices correspond to which vector component is non zero

  // set the transform values using the angle vector
  _t->setTransformPPValues(angles);
  _t->execute();
}

#endif
