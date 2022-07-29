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

#include "NekSideSpatialBinUserObject.h"

/**
 * Compute a side integral of the NekRS solution in spatial bins.
 */
class NekBinnedSideIntegral : public NekSideSpatialBinUserObject
{
public:
  static InputParameters validParams();

  NekBinnedSideIntegral(const InputParameters & parameters);

  virtual void executeUserObject() override;

  virtual void getBinVolumes() override;

  Real spatialValue(const Point & p, const unsigned int & component) const override;

  /**
   * Compute the integral over the side bins
   * @param[in] integrand field to integrate
   * @param[out] total_integral integral over each bin
   */
  virtual void binnedSideIntegral(const field::NekFieldEnum & integrand, double * total_integral);

  /**
   * Compute the integrals
   */
  virtual void computeIntegral();
};
