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

#include "NekVolumeSpatialBinUserObject.h"

/**
 * Compute a volume integral of the NekRS solution in spatial bins.
 */
class NekBinnedVolumeIntegral : public NekVolumeSpatialBinUserObject
{
public:
  static InputParameters validParams();

  NekBinnedVolumeIntegral(const InputParameters & parameters);

  virtual Real spatialValue(const Point & p, const unsigned int & component) const override;

  virtual void execute() override;

  virtual void getBinVolumes() override;

  /**
   * Compute a volume integral over the bins
   * @param[in] integrand field to integrate
   * @param[out] total_integral integrated values in each bin
   */
  virtual void binnedVolumeIntegral(const field::NekFieldEnum & integrand, double * total_integral);
};
