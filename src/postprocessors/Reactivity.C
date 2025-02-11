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

#include "Reactivity.h"
#include "openmc/eigenvalue.h"

registerMooseObject("CardinalApp", Reactivity);

InputParameters
Reactivity::validParams()
{
  InputParameters params = OpenMCPostprocessor::validParams();
  params.addClassDescription("Calculate the reactivity based on the combined k-effective value.");
  return params;
}

Reactivity::Reactivity(const InputParameters & parameters)
  : OpenMCPostprocessor(parameters)
{
  if (openmc::settings::run_mode != openmc::RunMode::EIGENVALUE)
    mooseError("Reactivity can only be computed when running OpenMC in eigenvalue mode!");
}

Real
Reactivity::getValue() const
{
  int n = openmc::simulation::n_realizations;

  if (n <= 3)
    mooseError("Cannot compute reactivity with fewer than 4 realizations!");

  double k_eff[2];
  openmc::openmc_get_keff(k_eff);
  double combined_k_effective = k_eff[0];

  return (combined_k_effective - 1.0) / combined_k_effective;
}
