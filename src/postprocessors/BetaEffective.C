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

#include "BetaEffective.h"

#include "OpenMCProblemBase.h"

registerMooseObject("CardinalApp", BetaEffective);

InputParameters
BetaEffective::validParams()
{
  auto params = GeneralPostprocessor::validParams();
  params += OpenMCBase::validParams();
  params.addClassDescription("A post-processor which computes and returns the kinetics parameter $\\beta_{eff}$.");

  return params;
}

BetaEffective::BetaEffective(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    OpenMCBase(this, parameters)
{
  if (openmc::settings::run_mode != openmc::RunMode::EIGENVALUE)
    mooseError("Beta effective is only computed when running OpenMC in eigenvalue mode!");
}

Real
BetaEffective::getValue() const
{
  const auto & ifp_tally = _openmc_problem->getKineticsParamTally();

  const Real num = xt::view(ifp_tally.results_, xt::all(), 1, static_cast<int>(openmc::TallyResult::SUM))[0] / ifp_tally.n_realizations_;
  const Real den = xt::view(ifp_tally.results_, xt::all(), 2, static_cast<int>(openmc::TallyResult::SUM))[0] / ifp_tally.n_realizations_;

  return num / den;
}

#endif
