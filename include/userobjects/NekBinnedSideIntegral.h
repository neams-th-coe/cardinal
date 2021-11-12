#pragma once

#include "NekSideSpatialBinUserObject.h"

/**
 * Compute a side integral of the NekRS solution in spatial bins.
 */
class NekBinnedSideIntegral : public NekSideSpatialBinUserObject
{
public:
  static InputParameters validParams();

  NekBinnedSideIntegral(const InputParameters & parameters);

  virtual void execute() override;

  virtual void getBinVolumes() override;

  Real spatialValue(const Point & p, const unsigned int & component) const override;
};
