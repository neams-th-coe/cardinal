#pragma once

#include "NekSpatialBinUserObject.h"

/**
 * Compute a volume integral of the NekRS solution in spatial bins.
 */
class NekBinnedVolumeIntegral : public NekSpatialBinUserObject
{
public:
  static InputParameters validParams();

  NekBinnedVolumeIntegral(const InputParameters & parameters);

  ~NekBinnedVolumeIntegral();

  virtual void execute() override;

protected:
  /// Volumes of each bin
  double * _bin_volumes;
};
