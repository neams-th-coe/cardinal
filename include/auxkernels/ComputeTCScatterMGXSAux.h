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

#pragma once

#include "OpenMCAuxKernel.h"

/**
 * This auxkernel computes an transport-corrected multi-group
 * cross section given a group-wise scattering reaction rate,
 * and the outgoing sum of group-wise P1 scattering reaction
 * rates, and the scalar flux.
 */
class ComputeTCScatterMGXSAux : public OpenMCAuxKernel
{
public:
  static InputParameters validParams();

  ComputeTCScatterMGXSAux(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;

  /// The group-wise P0 scattering reaction rate for computing the MGXS.
  const VariableValue & _p0_scattering_rates;

  /// The group-wise P1 scattering reaction rates for computing the MGXS.
  std::vector<const VariableValue *> _p1_scattering_rates;

  /// The group-wise scalar flux for computing the MGXS.
  const VariableValue & _scalar_flux;
};
