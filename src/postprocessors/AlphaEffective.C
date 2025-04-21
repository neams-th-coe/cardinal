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

#include "AlphaEffective.h"

#include "OpenMCProblemBase.h"

registerMooseObject("CardinalApp", AlphaEffective);

InputParameters
AlphaEffective::validParams()
{
  auto params = KEigenvalue::validParams();
  params.addClassDescription("A post-processor which computes and returns the kinetics parameter $\\Alpha_{eff}$.");

  return params;
}

AlphaEffective::AlphaEffective(const InputParameters & parameters)
  : KEigenvalue(parameters)
{
  if (!_openmc_problem->computeKineticsParams())
    mooseError("AlphaEffective can only be used if the OpenMC problem is computing kinetics parameters!");
}

Real
AlphaEffective::getValue() const
{
  const auto & ifp_tally = _openmc_problem->getKineticsParamTally();

  const Real num = xt::view(ifp_tally.results_, xt::all(), 0, static_cast<int>(openmc::TallyResult::SUM))[0] / ifp_tally.n_realizations_;
  const Real den = xt::view(ifp_tally.results_, xt::all(), 2, static_cast<int>(openmc::TallyResult::SUM))[0] / ifp_tally.n_realizations_;

  return num / den / KEigenvalue::getValue();
}

#endif
