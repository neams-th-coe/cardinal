#pragma once

#include "NekSideFieldPostprocessor.h"
#include "CardinalEnums.h"

/**
 * Compute the extreme value of a specified quantity over the boundaries
 * in the nekRS mesh.
 *
 * Note that this calculation is done directly on the mesh that nekRS solves on,
 * _not_ the mesh created for solution transfer in NekRSMesh.
 */
class NekSideExtremeValue : public NekSideFieldPostprocessor
{
public:
  static InputParameters validParams();

  NekSideExtremeValue(const InputParameters & parameters);

  virtual Real getValue() override;

protected:
  /// type of extrema operation
  const operation::OperationEnum _type;
};

