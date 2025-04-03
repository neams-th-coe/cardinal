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

#ifdef ENABLE_OPENMC_COUPLING

#include "ParticleFilter.h"

#include "CardinalEnums.h"

#include "openmc/tallies/filter_particle.h"

registerMooseObject("CardinalApp", ParticleFilter);

InputParameters
ParticleFilter::validParams()
{
  auto params = FilterBase::validParams();
  params.addClassDescription(
      "A class which provides a thin wrapper around an OpenMC ParticleFilter.");
  params.addRequiredParam<MultiMooseEnum>("particles", getParticleFilterEnums(), "The particles to filter for.");

  return params;
}

ParticleFilter::ParticleFilter(const InputParameters & parameters)
  : FilterBase(parameters)
{
  for (const auto & m_enum : getParam<MultiMooseEnum>("particles"))
  {
    _particles.emplace_back(m_enum);

    // Need to convert MultiMooseEnum values (all upper case characters) to lower case characters.
    std::transform(_particles.back().begin(), _particles.back().end(), _particles.back().begin(), [](unsigned char c) { return std::tolower(c); });
  }
  auto particle_filter = dynamic_cast<openmc::ParticleFilter *>(openmc::Filter::create("particle"));

  std::vector<openmc::ParticleType> p;
  for (const auto & ps : _particles)
    p.emplace_back(openmc::str_to_particle_type(ps));

  particle_filter->set_particles(p);
  _filter = particle_filter;
}

std::string
ParticleFilter::binName(unsigned int bin_index) const
{
  return _particles[bin_index];
}

#endif
