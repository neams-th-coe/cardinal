#pragma once

#include "OpenMCPostprocessor.h"
#include "CardinalEnums.h"

class KStandardDeviation;

template <>
InputParameters validParams<KStandardDeviation>();

/**
 * Get the k-effective eigenvalue standard deviation computed by OpenMC.
 */
class KStandardDeviation : public OpenMCPostprocessor
{
public:
  static InputParameters validParams();

  KStandardDeviation(const InputParameters & parameters);

  virtual Real getValue() override;

protected:
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
