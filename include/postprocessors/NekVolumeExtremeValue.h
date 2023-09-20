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

  virtual Real getValue() const override;

protected:
  /// type of extrema operation
  const operation::OperationEnum _type;
};
