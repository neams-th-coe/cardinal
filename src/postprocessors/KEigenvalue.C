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

#include "KEigenvalue.h"
#include "openmc/eigenvalue.h"
#include "openmc/math_functions.h"
#include "openmc/constants.h"

registerMooseObject("CardinalApp", KEigenvalue);

InputParameters
KEigenvalue::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  params += OpenMCBase::validParams();
  params.addClassDescription("k eigenvalue computed by OpenMC");
  params.addParam<MooseEnum>("value_type",
                             getEigenvalueEnum(),
                             "Type of eigenvalue global tally to report");
  params.addParam<MooseEnum>(
      "output",
      getStatsOutputEnum(),
      "The value to output. Options are $k_{eff}$ (mean), the standard deviation "
      "of $k_{eff}$ (std_dev), or the relative error of $k_{eff}$ (rel_err).");
  return params;
}

KEigenvalue::KEigenvalue(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    OpenMCBase(this, parameters),
    _type(getParam<MooseEnum>("value_type").getEnum<eigenvalue::EigenvalueEnum>()),
    _output(getParam<MooseEnum>("output").getEnum<statistics::OutputEnum>())
{
  if (openmc::settings::run_mode != openmc::RunMode::EIGENVALUE)
    mooseError("Eigenvalues are only computed when running OpenMC in eigenvalue mode!");
}

Real
KEigenvalue::getValue() const
{
  switch (_output)
  {
    case statistics::OutputEnum::Mean:
      return kMean();

    case statistics::OutputEnum::StDev:
      return KStandardDeviation();

    case statistics::OutputEnum::RelError:
      return kRelativeError();

    default:
      mooseError("Internal error: Unhandled statistics::OutputEnum enum in KEigenvalue.");
      break;
  }
}

Real
KEigenvalue::kMean() const
{
  int n = openmc::simulation::n_realizations;
  const auto & gt = openmc::simulation::global_tallies;

  switch (_type)
  {
    case eigenvalue::collision:
      return gt(openmc::GlobalTally::K_COLLISION, openmc::TallyResult::SUM) / n;
    case eigenvalue::absorption:
      return gt(openmc::GlobalTally::K_ABSORPTION, openmc::TallyResult::SUM) / n;
    case eigenvalue::tracklength:
      return gt(openmc::GlobalTally::K_TRACKLENGTH, openmc::TallyResult::SUM) / n;
    case eigenvalue::combined:
    {
      if (n <= 3)
        mooseError("Cannot compute combined k-effective estimate with fewer than 4 realizations!\n"
                   "Please change the 'value_type' to either 'collision', 'tracklength', or "
                   "'absorption'.");

      double k_eff[2];
      openmc::openmc_get_keff(k_eff);
      return k_eff[0];
    }
    default:
      mooseError("Internal error: Unhandled EigenvalueEnum in KEigenvalue!");
  }
}

Real
KEigenvalue::KStandardDeviation() const
{
  const auto & gt = openmc::simulation::global_tallies;
  int n = openmc::simulation::n_realizations;

  double t_n1 = 1.0;
  if (openmc::settings::confidence_intervals)
  {
    double alpha = 1.0 - openmc::CONFIDENCE_LEVEL;
    t_n1 = openmc::t_percentile(1.0 - alpha / 2.0, n - 1);
  }

  switch (_type)
  {
    case eigenvalue::collision:
    {
      double mean = gt(openmc::GlobalTally::K_COLLISION, openmc::TallyResult::SUM) / n;
      double sum_sq = gt(openmc::GlobalTally::K_COLLISION, openmc::TallyResult::SUM_SQ);
      return t_n1 * stdev(mean, sum_sq, openmc::simulation::n_realizations);
    }
    case eigenvalue::absorption:
    {
      double mean = gt(openmc::GlobalTally::K_ABSORPTION, openmc::TallyResult::SUM) / n;
      double sum_sq = gt(openmc::GlobalTally::K_ABSORPTION, openmc::TallyResult::SUM_SQ);
      return t_n1 * stdev(mean, sum_sq, openmc::simulation::n_realizations);
    }
    case eigenvalue::tracklength:
    {
      double mean = gt(openmc::GlobalTally::K_TRACKLENGTH, openmc::TallyResult::SUM) / n;
      double sum_sq = gt(openmc::GlobalTally::K_TRACKLENGTH, openmc::TallyResult::SUM_SQ);
      return t_n1 * stdev(mean, sum_sq, openmc::simulation::n_realizations);
    }
    case eigenvalue::combined:
    {
      if (n <= 3)
        mooseError("Cannot compute combined k-effective standard deviation with fewer than 4 "
                   "realizations!");

      double k_eff[2];
      openmc::openmc_get_keff(k_eff);
      return k_eff[1];
    }
    default:
      mooseError("Internal error: Unhandled StandardDeviationEnum in KEigenvalue!");
  }
}

Real
KEigenvalue::kRelativeError() const
{
  const auto mean = kMean();
  return mean > 0.0 ? KStandardDeviation() / kMean() : 0.0;
}

#endif
