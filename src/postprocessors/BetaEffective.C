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
  params.addClassDescription(
      "A post-processor which computes and returns the kinetics parameter $\\beta_{eff}$.");
  params.addParam<MooseEnum>(
      "output",
      getKineticsOutputEnum(),
      "The value to output. Options are $\\beta_{eff}$ (val), the standard deviation "
      "of $\\beta_{eff}$ (std_dev), or the relative error of $\\beta_{eff}$ (rel_err).");

  return params;
}

BetaEffective::BetaEffective(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    OpenMCBase(this, parameters),
    _output(getParam<MooseEnum>("output").getEnum<kinetics::KineticsOutputEnum>())
{
  if (!_openmc_problem->computeKineticsParams())
    mooseError(
        "BetaEffective can only be used if the OpenMC problem is computing kinetics parameters!");
}

Real
BetaEffective::getValue() const
{
  const auto & ifp_tally = _openmc_problem->getKineticsParamTally();
  const auto n = ifp_tally.n_realizations_;

  const auto num_sum =
      xt::view(ifp_tally.results_, xt::all(), 1, static_cast<int>(openmc::TallyResult::SUM));
  const auto den_sum =
      xt::view(ifp_tally.results_, xt::all(), 2, static_cast<int>(openmc::TallyResult::SUM));

  const auto num_ss =
      xt::view(ifp_tally.results_, xt::all(), 1, static_cast<int>(openmc::TallyResult::SUM_SQ));
  const auto den_ss =
      xt::view(ifp_tally.results_, xt::all(), 2, static_cast<int>(openmc::TallyResult::SUM_SQ));

  const Real beta_eff = (num_sum[0] / n) / (den_sum[0] / n);

  const Real num_rel = _openmc_problem->relativeError(num_sum, num_ss, n)[0];
  const Real den_rel = _openmc_problem->relativeError(den_sum, den_ss, n)[0];
  const Real beta_eff_rel = std::sqrt(num_rel * num_rel + den_rel * den_rel);

  switch (_output)
  {
    case kinetics::KineticsOutputEnum::Value:
      return beta_eff;

    case kinetics::KineticsOutputEnum::StDev:
      return beta_eff * beta_eff_rel;

    case kinetics::KineticsOutputEnum::RelError:
      return beta_eff_rel;

    default:
      mooseError("Internal error: Unhandled kinetics::KineticsOutputEnum.");
      break;
  }

  return beta_eff;
}

#endif
