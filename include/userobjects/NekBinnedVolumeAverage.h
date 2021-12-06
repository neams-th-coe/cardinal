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

#include "NekBinnedVolumeIntegral.h"

/**
 * Compute a volume average of the NekRS solution in spatial bins.
 */
class NekBinnedVolumeAverage : public NekBinnedVolumeIntegral
{
public:
  static InputParameters validParams();

  NekBinnedVolumeAverage(const InputParameters & parameters);

  virtual void execute() override;
};
