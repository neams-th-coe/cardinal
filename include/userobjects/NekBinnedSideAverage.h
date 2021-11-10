#pragma once

#include "NekBinnedSideIntegral.h"

/**
 * Compute a side average of the NekRS solution in spatial bins.
 */
class NekBinnedSideAverage : public NekBinnedSideIntegral
{
public:
  static InputParameters validParams();

  NekBinnedSideAverage(const InputParameters & parameters);

  virtual void execute() override;
};
