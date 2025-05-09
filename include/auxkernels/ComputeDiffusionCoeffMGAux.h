#pragma once

#include "OpenMCAuxKernel.h"

/**
 * This auxkernel computes multi-group particle diffusion coefficients
 * given a group-wise total reaction rate, the outgoing sum of group-wise
 * P1 scattering reaction rates, and the group-wise scalar flux.
 */
class ComputeDiffusionCoeffMGAux : public OpenMCAuxKernel
{
public:
  static InputParameters validParams();

  ComputeDiffusionCoeffMGAux(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;

  /// The value the diffusion coefficient should take in a void region.
  const Real & _void_diff;

  /// The group-wise total reaction rate for computing the MG diffusion coefficient.
  const VariableValue & _total_rxn_rate;

  /// The group-wise P1 scattering reaction rates for computing the MG diffusion coefficient.
  std::vector<const VariableValue *> _p1_scattering_rates;

  /// The group-wise scalar flux for computing the MG diffusion coefficient.
  const VariableValue & _scalar_flux;
};
