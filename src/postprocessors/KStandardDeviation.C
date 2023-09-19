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

#include "KStandardDeviation.h"
#include "openmc/eigenvalue.h"
#include "openmc/math_functions.h"
#include "openmc/constants.h"

registerMooseObject("CardinalApp", KStandardDeviation);

InputParameters
KStandardDeviation::validParams()
{
  InputParameters params = OpenMCPostprocessor::validParams();
  params.addParam<MooseEnum>("value_type",
                             getEigenvalueEnum(),
                             "Type of eigenvalue global tally to report");
  params.addClassDescription("Extract the k standard deviation computed by OpenMC");
  return params;
}

KStandardDeviation::KStandardDeviation(const InputParameters & parameters)
  : OpenMCPostprocessor(parameters),
    _type(getParam<MooseEnum>("value_type").getEnum<eigenvalue::EigenvalueEnum>())
{
  if (openmc::settings::run_mode != openmc::RunMode::EIGENVALUE)
    mooseError("Eigenvalues are only computed when running OpenMC in eigenvalue mode!");
}

Real
KStandardDeviation::stdev(const double & mean, const double & sum_sq) const
{
  int n = openmc::simulation::n_realizations;
  return n > 1 ? std::sqrt(std::max(0.0, (sum_sq / n - mean * mean) / (n - 1))) : 0.0;
}

Real
KStandardDeviation::getValue() const
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
      return t_n1 * stdev(mean, sum_sq);
    }
    case eigenvalue::absorption:
    {
      double mean = gt(openmc::GlobalTally::K_ABSORPTION, openmc::TallyResult::SUM) / n;
      double sum_sq = gt(openmc::GlobalTally::K_ABSORPTION, openmc::TallyResult::SUM_SQ);
      return t_n1 * stdev(mean, sum_sq);
    }
    case eigenvalue::tracklength:
    {
      double mean = gt(openmc::GlobalTally::K_TRACKLENGTH, openmc::TallyResult::SUM) / n;
      double sum_sq = gt(openmc::GlobalTally::K_TRACKLENGTH, openmc::TallyResult::SUM_SQ);
      return t_n1 * stdev(mean, sum_sq);
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
      mooseError("Unhandled StandardDeviationEnum in KStandardDeviation!");
  }
}

#endif
