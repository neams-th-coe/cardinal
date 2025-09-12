#pragma once

#include "FilterBase.h"

/**
 * A class which provides a thin wrapper around an OpenMC ParticleFilter
 * for use by Cardinal mapped tallies.
 */
class ParticleFilter : public FilterBase
{
public:
  static InputParameters validParams();

  ParticleFilter(const InputParameters & parameters);

  /**
   * A function which returns the short-form name for each bin of
   * this filter. Used to label auxvariables a TallyBase scores in.
   * ParticleFilter(s) uses different names depending on the particle.
   * @param[in] the bin index
   * @return a short name for the bin represented by bin_index
   */
  virtual std::string binName(unsigned int bin_index) const override;

private:
  /// The particles to filter for.
  std::vector<std::string> _particles;
};
