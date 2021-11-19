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

  /**
   * Compute the integral over the side bins
   * @param[in] integrand field to integrate
   * @param[out] total_integral integral over each bin
   */
  virtual void binnedSideIntegral(const field::NekFieldEnum & integrand, double * total_integral);
};
