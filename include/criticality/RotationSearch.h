#pragma once

#include "CriticalitySearchBase.h"
#include "OpenMCCellTransform.h"
#include "CardinalEnums.h"
#include "UserObjectInterface.h"

/**
 * Perform a criticality search based on a rotation angle
 */
class RotationSearch : public CriticalitySearchBase,
                       public UserObjectInterface,
                       public PostprocessorInterface
{
public:
  static InputParameters validParams();

  RotationSearch(const InputParameters & parameters);

  /** Verifies that the definition of the OpenMCCellTransform
   * UserObject is valid for a rotational criticality search
   */
  void checkValidVectorValueForRotationSearch();

  virtual void updateOpenMCModel(const Real & input) override;

protected:
  virtual std::string units() const override { return "[degrees]"; }
  virtual std::string quantity() const override { return "UO: [" + _transform_name + "]"; }

  /// name of the OpenMCCellTransform UserObject used to control the criticality search
  const UserObjectName _transform_name;

  /// the index of the rotational axis used to search for criticality
  const int _rotation_axis_idx;

  /// pointer to OpenMCCellTransform to update the initial critical guess each iteration
  OpenMCCellTransform * _t;
};
