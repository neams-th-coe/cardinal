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
