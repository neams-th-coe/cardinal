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

registerMooseObject("CardinalApp", KEigenvalue);

InputParameters
KEigenvalue::validParams()
{
  InputParameters params = OpenMCPostprocessor::validParams();
  params.addParam<MooseEnum>("value_type",
                             getEigenvalueEnum(),
                             "Type of eigenvalue global tally to report");
  params.addClassDescription("Extract the k eigenvalue computed by OpenMC");
  return params;
}

KEigenvalue::KEigenvalue(const InputParameters & parameters)
  : OpenMCPostprocessor(parameters),
    _type(getParam<MooseEnum>("value_type").getEnum<eigenvalue::EigenvalueEnum>())
{
  if (openmc::settings::run_mode != openmc::RunMode::EIGENVALUE)
    mooseError("Eigenvalues are only computed when running OpenMC in eigenvalue mode!");
}

Real
KEigenvalue::getValue() const
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
          "Please change the 'value_type' to either 'collision', 'tracklength', or 'absorption'.");

      double k_eff[2];
      openmc::openmc_get_keff(k_eff);
      return k_eff[0];
    }
    default:
      mooseError("Unhandled EigenvalueEnum in KEigenvalue!");
  }
}

#endif
