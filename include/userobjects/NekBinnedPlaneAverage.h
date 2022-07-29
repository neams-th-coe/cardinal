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

#include "NekBinnedPlaneIntegral.h"

/**
 * Compute a side average of the NekRS solution in spatial bins.
 */
class NekBinnedPlaneAverage : public NekBinnedPlaneIntegral
{
public:
  static InputParameters validParams();

  NekBinnedPlaneAverage(const InputParameters & parameters);

  virtual void executeUserObject() override;
};
