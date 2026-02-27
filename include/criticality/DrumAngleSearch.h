#pragma once

#include "CriticalitySearchBase.h"
#include "OpenMCCellTransform.h"
#include "CardinalEnums.h"

/**
 * Perform a criticality search based  Pon a material
 */
class DrumAngleSearch : public CriticalitySearchBase
{
public:
  static InputParameters validParams();

  DrumAngleSearch(const InputParameters & parameters);

  virtual void updateOpenMCModel(const Real & input) override;

protected:
  virtual std::string units() const override { return "[degrees]"; }

  // name of the OpenMCCellTransform UserObject used to control the criticality search
  const std::string _transform_name;

  // the rotational axis used to search for criticality
  const MooseEnum _rotation_axis_char;

  /// pointer to OpenMCCellTransform to update the initial critical guess each iteration
  const OpenMCCellTransform * _t;
};
