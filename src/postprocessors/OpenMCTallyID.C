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

#include "OpenMCTallyID.h"

#include "openmc/tallies/tally.h"

registerMooseObject("CardinalApp", OpenMCTallyID);

InputParameters
OpenMCTallyID::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  MooseEnum type("instantaneous total", "total");
  params.addParam<MooseEnum>("value_type", type,
      "How to report the number of particles; either instantaneous (the value used "
      "in the most recent solve) or total (accumulated over all previous Picard "
      "iterations");

  params.addClassDescription("Number of particles transported by OpenMC");
  return params;
}

OpenMCTallyID::OpenMCTallyID(const InputParameters & parameters)
  : GeneralPostprocessor(parameters) {}

Real
OpenMCTallyID::getValue() const
{
  auto max_it = std::max_element(openmc::model::tallies.begin(), openmc::model::tallies.end(),
         [](const auto & a, const auto & b) { return a->id() < b->id(); });
  return (*max_it)->id();
}

#endif
