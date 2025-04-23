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
  auto params = GeneralPostprocessor::validParams();
  params += OpenMCBase::validParams();
  params.addClassDescription(
      "A post-processor which computes and returns the kinetics parameter $\\Lambda_{eff}$.");
  params.addParam<MooseEnum>(
      "output",
      getKineticsOutputEnum(),
      "The value to output. Options are $\\Lambda_{eff}$ (val), the standard deviation "
      "of $\\Lambda_{eff}$ (std_dev), or the relative error of $\\Lambda_{eff}$ (rel_err).");

  return params;
}

LambdaEffective::LambdaEffective(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    OpenMCBase(this, parameters),
    _output(getParam<MooseEnum>("output").getEnum<kinetics::KineticsOutputEnum>())
{
  if (!_openmc_problem->computeKineticsParams())
    mooseError(
        "LambdaEffective can only be used if the OpenMC problem is computing kinetics parameters!");
}

Real
LambdaEffective::getValue() const
{
  const auto & ifp_tally = _openmc_problem->getKineticsParamTally();
  const auto n = ifp_tally.n_realizations_;

  const Real num_mean =
      xt::view(ifp_tally.results_, xt::all(), 0, static_cast<int>(openmc::TallyResult::SUM))[0] / n;
  const Real den_mean =
      xt::view(ifp_tally.results_, xt::all(), 2, static_cast<int>(openmc::TallyResult::SUM))[0] / n;

  const Real num_ss =
      xt::view(ifp_tally.results_, xt::all(), 0, static_cast<int>(openmc::TallyResult::SUM_SQ))[0];
  const Real den_ss =
      xt::view(ifp_tally.results_, xt::all(), 2, static_cast<int>(openmc::TallyResult::SUM_SQ))[0];

  double k_eff[2];
  openmc::openmc_get_keff(k_eff);
  const Real k_rel = k_eff[1] / k_eff[0];

  const Real lambda_eff = num_mean / den_mean / k_eff[0];

  const Real num_rel = relerr(num_mean, num_ss, n);
  const Real den_rel = relerr(den_mean, den_ss, n);
  const Real lambda_rel = std::sqrt(num_rel * num_rel + den_rel * den_rel + k_rel * k_rel);

  switch (_output)
  {
    case kinetics::KineticsOutputEnum::Value:
      return lambda_eff;

    case kinetics::KineticsOutputEnum::StDev:
      return lambda_eff * lambda_rel;

    case kinetics::KineticsOutputEnum::RelError:
      return lambda_rel;

    default:
      mooseError("Internal error: Unhandled kinetics::KineticsOutputEnum.");
      break;
  }

  return lambda_eff;
}

#endif
