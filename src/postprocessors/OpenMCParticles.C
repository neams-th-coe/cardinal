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

#include "OpenMCParticles.h"

registerMooseObject("CardinalApp", OpenMCParticles);

InputParameters
OpenMCParticles::validParams()
{
  InputParameters params = OpenMCPostprocessor::validParams();
  MooseEnum type("instantaneous total", "total");
  params.addParam<MooseEnum>("value_type", type,
      "How to report the number of particles; either instantaneous (the value used "
      "in the most recent solve) or total (accumulated over all previous Picard "
      "iterations");

  params.addClassDescription("Number of particles transported by OpenMC");
  return params;
}

OpenMCParticles::OpenMCParticles(const InputParameters & parameters)
  : OpenMCPostprocessor(parameters),
    _type(getParam<MooseEnum>("value_type"))
{
}

Real
OpenMCParticles::getValue() const
{
  switch (_type)
  {
    case 0:
      return openmc::settings::n_batches * openmc::settings::n_particles;
    case 1:
      return openmc::settings::n_batches * _openmc_problem->nTotalParticles();
    default:
      mooseError("Unhandled type enum in OpenMCParticles!");
  }
}

#endif
