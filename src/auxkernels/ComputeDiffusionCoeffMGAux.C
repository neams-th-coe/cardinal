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

#include "ComputeDiffusionCoeffMGAux.h"

registerMooseObject("CardinalApp", ComputeDiffusionCoeffMGAux);

InputParameters
ComputeDiffusionCoeffMGAux::validParams()
{
  auto params = OpenMCAuxKernel::validParams();
  params.addClassDescription(
      "An auxkernel that computes a multi-group diffusion coefficient using a group-wise total "
      "reaction rate, a list of group-wise P1 scattering reaction rates, and the group-wise "
      "scalar flux. This is intended to be added by the MGXS action.");
  params.addRequiredCoupledVar(
      "total_rxn_rate",
      "The total reaction rates to use for computing the multi-group diffusion coefficient.");
  params.addRequiredCoupledVar("p1_scatter_rxn_rates",
                               "The P1 group-wise scattering reaction rates to use for computing "
                               "the multi-group diffusion coefficient.");
  params.addRequiredCoupledVar(
      "scalar_flux",
      "The group-wise scalar flux used for computing the multi-group diffusion coefficient.");
  params.addParam<Real>("void_diffusion_coefficient",
                        1e3,
                        "The value the diffusion coefficient should take in a void region.");

  return params;
}

ComputeDiffusionCoeffMGAux::ComputeDiffusionCoeffMGAux(const InputParameters & parameters)
  : OpenMCAuxKernel(parameters),
    _void_diff(getParam<Real>("void_diffusion_coefficient")),
    _total_rxn_rate(coupledValue("total_rxn_rate")),
    _scalar_flux(coupledValue("scalar_flux"))
{
  for (unsigned int i = 0; i < coupledComponents("p1_scatter_rxn_rates"); ++i)
    _p1_scattering_rates.emplace_back(&coupledValue("p1_scatter_rxn_rates", i));
}

Real
ComputeDiffusionCoeffMGAux::computeValue()
{
  Real num = _total_rxn_rate[_qp];
  for (unsigned int g = 0; g < _p1_scattering_rates.size(); ++g)
    num -= (*_p1_scattering_rates[g])[_qp];

  const Real transport_xs = _scalar_flux[_qp] > 0.0 ? num / _scalar_flux[_qp] : 0.0;
  const Real diff_coeff =
      transport_xs > libMesh::TOLERANCE ? 1.0 / (3.0 * transport_xs) : _void_diff;
  return diff_coeff;
}

#endif
