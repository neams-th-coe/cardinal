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
 * A class which approximates gradients of constant monomial tallies using forward finite
 * differences. The gradient computation is based on:
 * K. N. Stolte and P. V. Tsvetkov (2023), Annals of Nuclear Energy, 182, 109617.
 * https://doi.org/10.1016/j.anucene.2022.109617
 */
class FDTallyGradAux : public OpenMCVectorAuxKernel
{
public:
  static InputParameters validParams();

  FDTallyGradAux(const InputParameters & parameters);

  /// We handle computing and storing the variable value manually.
  virtual void compute() override;

protected:
  /// Need to override computeValue() to avoid creating a pure-virtual class.
  virtual RealVectorValue computeValue() override { return RealVectorValue(0.0, 0.0, 0.0); }

  /// The external filter bin index for the score.
  const unsigned int _bin_index;

  /// The element's tally value.
  const VariableValue * _tally_val;

  /// The neighboring element's tally value.
  const VariableValue * _tally_neighbor_val;

  /**
   * The sum of outer products of y' with itself, where y' = x_j - x_i.
   * x_j is the centroid of the neighboring element j of element i.
   * x_i is the centroid of element i.
   */
  RealEigenMatrix _sum_y_y_t;

  /**
   * The sum of the finite difference approximation of u multiplied by y'.
   */
  RealEigenVector _sum_y_du_dy;
};
