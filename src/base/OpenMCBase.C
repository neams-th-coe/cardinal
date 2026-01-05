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

#include "OpenMCBase.h"
#include "openmc/eigenvalue.h"
#include "openmc/math_functions.h"
#include "libmesh/elem.h"

InputParameters
OpenMCBase::validParams()
{
  InputParameters params = emptyInputParameters();
  return params;
}

OpenMCBase::OpenMCBase(const ParallelParamObject * moose_object, const InputParameters & parameters)
  : _openmc_problem(
        dynamic_cast<OpenMCCellAverageProblem *>(&moose_object->getMooseApp().feProblem()))
{
  if (!_openmc_problem)
    mooseError(moose_object->type() +
               " can only be used with problems of type 'OpenMCCellAverageProblem'!");
}

Real
OpenMCBase::stdev(const double & mean, const double & sum_sq, unsigned int realizations) const
{
  return realizations > 1
             ? std::sqrt(std::max(0.0, (sum_sq / realizations - mean * mean) / (realizations - 1)))
             : 0.0;
}

Real
OpenMCBase::kMean(const eigenvalue::EigenvalueEnum estimator) const
{
  int n = openmc::simulation::n_realizations;
  const auto & gt = openmc::simulation::global_tallies;

  switch (estimator)
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
                   "Please change the estimator type to either 'collision', 'tracklength', or "
                   "'absorption'.");

      double k_eff[2];
      openmc::openmc_get_keff(k_eff);
      return k_eff[0];
    }
    default:
      mooseError("Internal error: Unhandled EigenvalueEnum!");
  }
}

Real
OpenMCBase::kStandardDeviation(const eigenvalue::EigenvalueEnum estimator) const
{
  const auto & gt = openmc::simulation::global_tallies;
  int n = openmc::simulation::n_realizations;

  double t_n1 = 1.0;
  if (openmc::settings::confidence_intervals)
  {
    double alpha = 1.0 - openmc::CONFIDENCE_LEVEL;
    t_n1 = openmc::t_percentile(1.0 - alpha / 2.0, n - 1);
  }

  switch (estimator)
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
      mooseError("Internal error: Unhandled StandardDeviationEnum!");
  }
}

#endif
