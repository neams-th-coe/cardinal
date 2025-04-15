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

class ComputeMGXSAux : public OpenMCAuxKernel
{
public:
  static InputParameters validParams();

  ComputeMGXSAux(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;

  /// The reaction rates for computing the MGXS.
  std::vector<const VariableValue *> _mg_reaction_rates;

  /// The normalization factor. Normally the group-wise scalar flux.
  std::vector<const VariableValue *> _norm_factors;
};
