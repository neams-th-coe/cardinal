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

#include "GeneralVectorPostprocessor.h"

/**
 * Returns a vector of velocity values corresponding to each mesh cell of
 * the OpenMC velocity field.
 */
class OpenMCVelocityField : public GeneralVectorPostprocessor
{
public:
  static InputParameters validParams();
  OpenMCVelocityField(const InputParameters & params);

  virtual void initialize() override;
  virtual void execute() override;
  virtual void finalize() override {}

private:
  /// Cell ID corresponding to each value in the OpenMC velocity field
  VectorPostprocessorValue & _cell_id;

  /// Velocity components assigned to each cell of the OpenMC velocity field
  VectorPostprocessorValue & _vx;
  VectorPostprocessorValue & _vy;
  VectorPostprocessorValue & _vz;
};
