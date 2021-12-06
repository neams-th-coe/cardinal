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

#include "NekBinnedSideIntegral.h"

/**
 * Compute an average of the NekRS solution in spatial bins on a sideset
 */
class NekBinnedSideAverage : public NekBinnedSideIntegral
{
public:
  static InputParameters validParams();

  NekBinnedSideAverage(const InputParameters & parameters);

  virtual void execute() override;
};
