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
      getStatsOutputEnum(),
      "The value to output. Options are $\\beta_{eff}$ (mean), the standard deviation "
      "of $\\beta_{eff}$ (std_dev), or the relative error of $\\beta_{eff}$ (rel_err).");

  return params;
}

BetaEffective::BetaEffective(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    OpenMCBase(this, parameters),
    _output(getParam<MooseEnum>("output").getEnum<statistics::OutputEnum>())
{
  if (!_openmc_problem->computeKineticsParams())
    mooseError(
        "BetaEffective can only be used if the OpenMC problem is computing kinetics parameters!");
}

Real
BetaEffective::getValue() const
{
  const auto & common_tally = _openmc_problem->getCommonKineticsTally();
  const auto & beta_tally = _openmc_problem->getBetaTally();

  const auto n_num = beta_tally.n_realizations_;
  const auto n_den = common_tally.n_realizations_;

  Real num_sum = 0.0;
  for (unsigned int i = 0; i < 6; ++i)
    num_sum += xt::view(beta_tally.results_, xt::all(), 0, static_cast<int>(openmc::TallyResult::SUM))[i];
  const auto den_sum =
      xt::view(common_tally.results_, xt::all(), 1, static_cast<int>(openmc::TallyResult::SUM))[0];

  Real num_ss = 0.0;
  for (unsigned int i = 0; i < 6; ++i)
    num_ss += xt::view(beta_tally.results_, xt::all(), 0, static_cast<int>(openmc::TallyResult::SUM_SQ))[i];
  const auto den_ss =
      xt::view(common_tally.results_, xt::all(), 1, static_cast<int>(openmc::TallyResult::SUM_SQ))[0];

  const Real beta_eff = (num_sum / n_num) / (den_sum / n_den);

  const Real num_rel = _openmc_problem->relativeError(num_sum, num_ss, n_num);
  const Real den_rel = _openmc_problem->relativeError(den_sum, den_ss, n_num);
  const Real beta_eff_rel = std::sqrt(num_rel * num_rel + den_rel * den_rel);

  switch (_output)
  {
    case statistics::OutputEnum::Mean:
      return beta_eff;

    case statistics::OutputEnum::StDev:
      return beta_eff * beta_eff_rel;

    case statistics::OutputEnum::RelError:
      return beta_eff_rel;

    default:
      mooseError("Internal error: Unhandled statistics::OutputEnum enum in BetaEffective.");
      break;
  }

  return beta_eff;
}

#endif
