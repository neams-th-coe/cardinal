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
};
