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

#include "ComputeTCScatterMGXSAux.h"

registerMooseObject("CardinalApp", ComputeTCScatterMGXSAux);

InputParameters
ComputeTCScatterMGXSAux::validParams()
{
  auto params = OpenMCAuxKernel::validParams();
  params.addClassDescription(
    "An auxkernel that computes a transport-corrected P0 scattering multi-group cross "
    "section using a list of scattering reaction rates and the group-wise scalar flux.");
  params.addRequiredCoupledVar(
    "p0_scatter_rxn_rate",
    "The P0 group-wise scattering reaction rates to use for computing the transport-corrected scattering cross section.");
  params.addRequiredCoupledVar(
    "p1_scatter_rxn_rates",
    "The P1 group-wise scattering reaction rates to use for computing the transport-corrected scattering cross section.");
  params.addRequiredCoupledVar(
    "scalar_flux",
    "The group-wise scalar flux used to compute the transport-corrected scattering cross section.");

  return params;
}

ComputeTCScatterMGXSAux::ComputeTCScatterMGXSAux(const InputParameters & parameters)
  : OpenMCAuxKernel(parameters),
    _p0_scattering_rates(coupledValue("p0_scatter_rxn_rate")),
    _scalar_flux(coupledValue("scalar_flux"))
{
  for (unsigned int i = 0; i < coupledComponents("p1_scatter_rxn_rates"); ++i)
    _p1_scattering_rates.emplace_back(&coupledValue("p1_scatter_rxn_rates", i));
}

Real
ComputeTCScatterMGXSAux::computeValue()
{
  Real num = _p0_scattering_rates[_qp];
  for (unsigned int g = 0; g < _p1_scattering_rates.size(); ++g)
    num -= (*_p1_scattering_rates[g])[_qp];

  return _scalar_flux[_qp] > 0.0 ? num / _scalar_flux[_qp] : 0.0;
}

#endif
