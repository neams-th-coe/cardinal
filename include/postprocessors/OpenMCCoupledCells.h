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

#include "OpenMCPostprocessor.h"

/**
 * Get the number of cells for which OpenMC is receiving temperature and/or
 * density feedback from MOOSE, or which are sending a cell tally to MOOSE.
 */
class OpenMCCoupledCells : public OpenMCPostprocessor
{
public:
  static InputParameters validParams();

  OpenMCCoupledCells(const InputParameters & parameters);

  virtual Real getValue() const override;
};
