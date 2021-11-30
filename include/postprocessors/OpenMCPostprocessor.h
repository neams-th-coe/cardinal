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

#include "GeneralPostprocessor.h"
#include "OpenMCCellAverageProblem.h"

/**
 * Base class for providing common information to postprocessors
 * operating directly on the OpenMC solution and geometry.
 */
class OpenMCPostprocessor : public GeneralPostprocessor
{
public:
  static InputParameters validParams();

  OpenMCPostprocessor(const InputParameters & parameters);

  virtual void initialize() override {}
  virtual void execute() override {}

protected:
  // Underlying problem
  const OpenMCCellAverageProblem * _openmc_problem;
};
