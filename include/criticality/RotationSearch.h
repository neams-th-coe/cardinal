#pragma once

#include "CriticalitySearchBase.h"
#include "OpenMCCellTransformBase.h"
#include "CardinalEnums.h"
#include "UserObjectInterface.h"

/**
 * Perform a criticality search based on a rotation angle
 */
class RotationSearch : public CriticalitySearchBase, public OpenMCCellTransformBase
{
public:
  static InputParameters validParams();

  RotationSearch(const InputParameters & parameters);

  /** Verifies that the definition of the OpenMCCellTransform
   * UserObject is valid for a rotational criticality search
   */

  virtual void updateOpenMCModel(const Real & angle) override;

protected:
  virtual std::string units() const override { return "[degrees]"; }
  virtual std::string quantity() const override { return "Rotation"; }

  /// the index of the rotational axis used to search for criticality
  const int _rotation_axis_idx;
};
