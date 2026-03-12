#pragma once

#include "CriticalitySearchBase.h"
#include "OpenMCCellTransform.h"
#include "CardinalEnums.h"
#include "UserObjectInterface.h"

/**
 * Perform a criticality search based  Pon a material
 */
class RotationSearch : public CriticalitySearchBase, public UserObjectInterface
{
public:
  static InputParameters validParams();

  RotationSearch(const InputParameters & parameters);

  virtual void updateOpenMCModel(const Real & input) override;

protected:
  virtual std::string units() const override { return "[degrees]"; }
  virtual std::string quantity() const override { return "UO: [" + _transform_name + "]"; }

  // name of the OpenMCCellTransform UserObject used to control the criticality search
  const UserObjectName _transform_name;

  // the rotational axis used to search for criticality
  const MooseEnum _rotation_axis_char;

  /// pointer to OpenMCCellTransform to update the initial critical guess each iteration
  OpenMCCellTransform * _t;
};
