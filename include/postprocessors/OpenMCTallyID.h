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

/**
 * Get the total number of particles simulated in OpenMC, i.e. the product
 * of the particles/batch multiplied by number of batches.
 */
class OpenMCTallyID : public GeneralPostprocessor
{
public:
  static InputParameters validParams();

  OpenMCTallyID(const InputParameters & parameters);

  virtual void initialize() override {}
  virtual void execute() override {}

  Real getValue() const;

protected:
  /**
   * How to report the particles, either as the values used in the most recent
   * Picard iteration, vs. a total accumulated over all previous OpenMC solves.
   */
  // const MooseEnum & _type;
};
