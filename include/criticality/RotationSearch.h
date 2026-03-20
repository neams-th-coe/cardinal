/********************************************************************/
/*                  SOFTWARE COPYRIGHT NOTIFICATION                 */
/*                             Cardinal                             */
/*                                                                  */
/*                  (c) 2021 UChicago Argonne, LLC                  */
/*                        ALL RIGHTS RESERVED                       */
/*                                                                  */
/*                 Prepared by UChicago Argonne, LLC                */
/*               Under Contract No. DE-AC02-06CH11357               */
/*                With the U. S. Department of Energy               */
/*                                                                  */
/*             Prepared by Battelle Energy Alliance, LLC            */
/*               Under Contract No. DE-AC07-05ID14517               */
/*                With the U. S. Department of Energy               */
/*                                                                  */
/*                 See LICENSE for full restrictions                */
/********************************************************************/

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
  // TODO: logan doesn't know what to put here
  virtual std::string quantity() const override { return "fixme"; }

  /// the index of the rotational axis used to search for criticality
  const int _rotation_axis_idx;
};
