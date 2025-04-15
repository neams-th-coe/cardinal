#pragma once

#include "OpenMCAuxKernel.h"

class ComputeDiffusionCoeffMGAux : public OpenMCAuxKernel
{
public:
  static InputParameters validParams();

  ComputeDiffusionCoeffMGAux(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;

  /// The group-wise total reaction rate for computing the MG diffusion coefficient.
  const VariableValue & _total_rxn_rate;

  /// The group-wise P1 scattering reaction rates for computing the MG diffusion coefficient.
  std::vector<const VariableValue *> _p1_scattering_rates;

  /// The group-wise scalar flux for computing the MG diffusion coefficient.
  const VariableValue & _scalar_flux;
};
