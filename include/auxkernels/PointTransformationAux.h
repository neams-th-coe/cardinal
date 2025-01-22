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

#include "AuxKernel.h"

#include "OpenMCBase.h"

/**
 * Auxkernel to display the reflected/rotated x, y, and z coordinates
 * that are used when mapping from the [Mesh] to OpenMC's geometry. This
 * can be used to ensure that you set the symmetry parameters on
 * OpenMCCellAverageProblem correctly
 */
class PointTransformationAux : public AuxKernel, public OpenMCBase
{
public:
  PointTransformationAux(const InputParameters & parameters);

  static InputParameters validParams();

protected:
  virtual Real computeValue();

  /// Coordinate component to visualize
  const int _d;
};
