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

class SpatialLegendreFilter;

/**
 * An auxkernel which evaluates a tally which has been expanded in Legendre
 * polynomials of position along a Cartesian axis by a SpatialLegendreFilter.
 * The expansion is reconstructed at each element centroid as
 *
 * \f$f(x) = \sum_{l=0}^{N}\frac{2l + 1}{x_{max} - x_{min}}a_l P_l(\hat{x})\f$
 *
 * where \f$a_l\f$ are the tally expansion coefficients stored in the tally
 * auxvariables, \f$P_l\f$ are the Legendre polynomials, and \f$\hat{x}\f$ is the
 * coordinate along the expansion axis normalized onto \f$[-1, 1]\f$.
 */
class SpatialLegendreAux : public OpenMCAuxKernel
{
public:
  static InputParameters validParams();

  SpatialLegendreAux(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;

  /// The SpatialLegendreFilter whose expansion this auxkernel evaluates.
  const SpatialLegendreFilter * _legendre_filter;

  /// The order of the Legendre expansion.
  unsigned int _order;

  /// The component of the axis the expansion is applied along (0 for x, 1 for y, 2 for z).
  unsigned int _axis;

  /**
   * NOTE: the expansion limits are deliberately NOT cached here. A filter using
   * 'use_displaced_mesh' re-derives them whenever the tallies are re-initialized, so they are read
   * from the filter on every evaluation in computeValue().
   */

  /// How each expansion coefficient is weighted when summing the Legendre series.
  enum class Normalization
  {
    VOLUMETRIC = 0,
    RAW = 1,
    NONE = 2
  };

  /// How each expansion coefficient is weighted when summing the Legendre series.
  const Normalization _normalization;

  /// The tally variables holding the Legendre expansion coefficients, indexed by Legendre order.
  std::vector<const VariableValue *> _coefficients;

  /// Scratch space for the Legendre polynomials evaluated at a point.
  std::vector<Real> _polynomials;
};
