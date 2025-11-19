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
      "beta_type",
      getBetaGroupEnum(),
      "The delayed group to calculate $\\beta_{eff}$ for. Options are groups 1-6 "
      "(from the ENDF delayed neutron groups) or the sum over all groups. Defaults "
      "to 'sum'.");
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
    _output(getParam<MooseEnum>("output").getEnum<statistics::OutputEnum>()),
    _beta_type(getParam<MooseEnum>("beta_type").getEnum<BetaTypeEnum>())
{
  if (!_openmc_problem->computeKineticsParams())
    mooseError(
        "BetaEffective can only be used if the OpenMC problem is computing kinetics parameters!");
}

Real
BetaEffective::getValue() const
{
  const auto & common_tally = _openmc_problem->getCommonKineticsTally();
  const auto & mg_beta = _openmc_problem->getMGBetaTally();

  unsigned int n_num = 0;
  Real num_sum = 0.0;
  Real num_ss = 0.0;
  switch (_beta_type)
  {
    case BetaTypeEnum::Sum:
      n_num = common_tally.n_realizations_;
      num_sum = xt::view(
          common_tally.results_, xt::all(), 2, static_cast<int>(openmc::TallyResult::SUM))[0];
      num_ss = xt::view(
          common_tally.results_, xt::all(), 2, static_cast<int>(openmc::TallyResult::SUM_SQ))[0];
      break;
    case BetaTypeEnum::D_1:
    case BetaTypeEnum::D_2:
    case BetaTypeEnum::D_3:
    case BetaTypeEnum::D_4:
    case BetaTypeEnum::D_5:
    case BetaTypeEnum::D_6:
      n_num = mg_beta.n_realizations_;
      num_sum = xt::view(
          mg_beta.results_,
          xt::all(),
          0,
          static_cast<int>(openmc::TallyResult::SUM))[static_cast<unsigned int>(_beta_type) - 1];
      num_ss = xt::view(
          mg_beta.results_,
          xt::all(),
          0,
          static_cast<int>(openmc::TallyResult::SUM_SQ))[static_cast<unsigned int>(_beta_type) - 1];
      break;
    default:
      mooseError("Internal error: Unknown BetaTypeEnum.");
      break;
  }

  const auto n_den = common_tally.n_realizations_;
  const auto den_sum =
      xt::view(common_tally.results_, xt::all(), 1, static_cast<int>(openmc::TallyResult::SUM))[0];
  const auto den_ss = xt::view(
      common_tally.results_, xt::all(), 1, static_cast<int>(openmc::TallyResult::SUM_SQ))[0];

  const Real beta_eff = (num_sum / n_num) / (den_sum / n_den);

  const Real num_rel = _openmc_problem->relativeError(num_sum, num_ss, n_num);
  const Real den_rel = _openmc_problem->relativeError(den_sum, den_ss, n_den);
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
