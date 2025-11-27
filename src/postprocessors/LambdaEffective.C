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

#include "LambdaEffective.h"

#include "OpenMCProblemBase.h"

#include "openmc/eigenvalue.h"

registerMooseObject("CardinalApp", LambdaEffective);

InputParameters
LambdaEffective::validParams()
{
  auto params = KEigenvalue::validParams();
  params.addClassDescription(
      "A post-processor which computes and returns the kinetics parameter $\\Lambda_{eff}$.");
  params.addParam<MooseEnum>(
      "output",
      getStatsOutputEnum(),
      "The value to output. Options are $\\Lambda_{eff}$ (mean), the standard deviation "
      "of $\\Lambda_{eff}$ (std_dev), or the relative error of $\\Lambda_{eff}$ (rel_err).");

  return params;
}

LambdaEffective::LambdaEffective(const InputParameters & parameters) : KEigenvalue(parameters)
{
  if (!_openmc_problem->computeKineticsParams())
    mooseError(
        "LambdaEffective can only be used if the OpenMC problem is computing kinetics parameters!");
}

Real
LambdaEffective::getValue() const
{
  const auto & ifp_tally = _openmc_problem->getCommonKineticsTally();
  const auto n = ifp_tally.n_realizations_;

  const auto num_sum =
      xt::view(ifp_tally.results_, xt::all(), 0, static_cast<int>(openmc::TallyResult::SUM));
  const auto den_sum =
      xt::view(ifp_tally.results_, xt::all(), 1, static_cast<int>(openmc::TallyResult::SUM));

  const auto num_ss =
      xt::view(ifp_tally.results_, xt::all(), 0, static_cast<int>(openmc::TallyResult::SUM_SQ));
  const auto den_ss =
      xt::view(ifp_tally.results_, xt::all(), 1, static_cast<int>(openmc::TallyResult::SUM_SQ));

  const auto mean_k = kMean();
  const auto k_rel = kRelativeError();

  const Real lambda_eff = (num_sum[0] / n) / (den_sum[0] / n) / mean_k;

  const Real num_rel = _openmc_problem->relativeError(num_sum, num_ss, n)[0];
  const Real den_rel = _openmc_problem->relativeError(den_sum, den_ss, n)[0];
  const Real lambda_rel = std::sqrt(num_rel * num_rel + den_rel * den_rel + k_rel * k_rel);

  switch (_output)
  {
    case statistics::OutputEnum::Mean:
      return lambda_eff;

    case statistics::OutputEnum::StDev:
      return lambda_eff * lambda_rel;

    case statistics::OutputEnum::RelError:
      return lambda_rel;

    default:
      mooseError("Internal error: Unhandled statistics::OutputEnum enum in LambdaEffective.");
      break;
  }

  return lambda_eff;
}

#endif
