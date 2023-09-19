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

#include "OpenMCPostprocessor.h"
#include "CardinalEnums.h"

/**
 * Get the k-effective eigenvalue standard deviation computed by OpenMC.
 */
class KStandardDeviation : public OpenMCPostprocessor
{
public:
  static InputParameters validParams();

  KStandardDeviation(const InputParameters & parameters);

  virtual Real getValue() const override;

protected:
  /**
   * Compute standard deviation
   * @param[in] mean mean
   * @param[in] sum_sq sum squared
   * @return standard deviation given N OpenMC realizations
   */
  Real stdev(const double & mean, const double & sum_sq) const;

  /**
   * Type of k-effective standard deviation value to report. Options:
   * collision, absorption, tracklength, and combined (default).
   *
   * The combined k-effective estimate is a minimum variance estimate
   * of k-effective based on a linear combination of the collision, absorption,
   * and tracklength estimates.
   */
  const eigenvalue::EigenvalueEnum _type;
};
