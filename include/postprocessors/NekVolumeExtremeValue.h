#pragma once

#include "NekFieldPostprocessor.h"
#include "CardinalEnums.h"

/**
 * Compute the extreme value of a specified quantity over the entire volume
 * of the nekRS mesh.
 *
 * Note that this calculation is done directly on the mesh that nekRS solves on,
 * _not_ the mesh created for solution transfer in NekRSMesh.
 */
class NekVolumeExtremeValue : public NekFieldPostprocessor
{
public:
  static InputParameters validParams();

  NekVolumeExtremeValue(const InputParameters & parameters);

  virtual Real getValue() override;

protected:
  /// type of extrema operation
  const operation::OperationEnum _type;
};

