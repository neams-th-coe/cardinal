#include "KStandardDeviation.h"
#include "openmc/eigenvalue.h"
#include "openmc/math_functions.h"
#include "openmc/constants.h"
#include "openmc/output.h"

registerMooseObject("CardinalApp", KStandardDeviation);

defineLegacyParams(KStandardDeviation);

InputParameters
KStandardDeviation::validParams()
{
  InputParameters params = OpenMCPostprocessor::validParams();
  params.addParam<MooseEnum>("value_type", getEigenvalueEnum(),
    "Type of eigenvalue global tally to report; options: collision, absorption, tracklength, combined (default)");
  params.addClassDescription("Extract the k standard deviation computed by OpenMC");
  return params;
}

KStandardDeviation::KStandardDeviation(const InputParameters & parameters) :
  OpenMCPostprocessor(parameters),
  _type(getParam<MooseEnum>("value_type").getEnum<eigenvalue::EigenvalueEnum>())
{
}

Real
KStandardDeviation::getValue()
{
  int n = openmc::simulation::n_realizations;
  const auto & gt = openmc::simulation::global_tallies;

  double mean, stdev;
  double alpha, t_n1, t_n3;
  if (openmc::settings::confidence_intervals)
  {
    alpha = 1.0 - openmc::CONFIDENCE_LEVEL;
    t_n1 = openmc::t_percentile(1.0 - alpha / 2.0, n - 1);
    t_n3 = openmc::t_percentile(1.0 - alpha / 2.0, n - 3);
  }
  else
  {
    t_n1 = 1.0;
    t_n3 = 1.0;
  }

  switch (_type)
  {
    case eigenvalue::collision:
    {
      std::tie(mean, stdev) = openmc::mean_stdev(&gt(openmc::GlobalTally::K_COLLISION, 0), n);
      return t_n1 * stdev;
    }
    case eigenvalue::absorption:
    {
      std::tie(mean, stdev) = openmc::mean_stdev(&gt(openmc::GlobalTally::K_ABSORPTION, 0), n);
      return t_n1 * stdev;
    }
    case eigenvalue::tracklength:
    {
      std::tie(mean, stdev) = openmc::mean_stdev(&gt(openmc::GlobalTally::K_TRACKLENGTH, 0), n);
      return t_n1 * stdev;
    }
    case eigenvalue::combined:
    {
      if (n <= 3)
        mooseError("Cannot compute combined k-effective standard deviation with fewer than 4 realizations!");

      double k_eff[2];
      openmc::openmc_get_keff(k_eff);
      return k_eff[1];
    }
    default:
      mooseError("Unhandled StandardDeviationEnum in KStandardDeviation!");
  }
}
