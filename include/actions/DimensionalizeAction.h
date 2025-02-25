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

#include "Action.h"

/**
 * This action establishes the scales for dimensionalizing the NekRS solution.
 * If nekRS is solving in non-dimensional form, this means that the nekRS solution
 * is performed for:
 *
 * - nondimensional temperature \f$T^\dagger\f$, defined as
 *   \f$T^\dagger=\frac{T-T_{ref}}{\Delta T_{ref}}\f$.
 *   The 'T' and 'dT' variables here represent these scales.
 *
 * - nondimensional velocity \f$U^\dagger=\frac{u}{U_{ref}}\f$. The 'U' variable here
 *   represents this velocity scale.
 *
 * - nondimensional pressure \f$P^dagger=\frac{P}{\rho_{0}U_{ref}^2}\f$. The 'rho'
 *   variable here represents this density parameter.
 *
 * In terms of heat flux boundary conditions, the entire energy conservation equation
 * in nekRS is multiplied by \f$\frac{L_{ref}}{\rho_{0}C_{p,0}U_{ref}\Delta T_{ref}}\f$
 * in order to clear the coefficient on the convective. Therefore, the heat source in
 * nekRS is expressed in nondimensional form as \f$q^\dagger=\frac{q}{\rho_{0}C_{p,0}U_{ref}\Delta
 * T_{ref}}\f$. Here, 'Cp' is the specific heat capacity scale.
 *
 * Unfortunately, there is no systematic way to get these reference
 * scales from nekRS, so we need to inform MOOSE of any possible scaling so that we
 * can appropriately scale the nekRS temperature to the dimensional form that is usually
 * expected in MOOSE. Therefore, these scales just need to match what is used in the nekRS
 * input files used to specify boundary conditions. Conversion between dimensional MOOSE
 * heat flux to dimensionless nekRS heat flux is performed automatically, and does not require
 * any special treatment in the nekRS scalarNeumannBC kernel.
 */
class DimensionalizeAction : public Action
{
public:
  static InputParameters validParams();

  DimensionalizeAction(const InputParameters & parameters);

  virtual void act() override;

protected:
  /// Reference characteristic velocity
  const Real & _U;

  /// Reference characteristic temperature
  const Real & _T;

  /// Reference characteristic temperature delta
  const Real & _dT;

  /// Reference characteristic length
  const Real & _L;

  /// Reference density
  const Real & _rho;

  /// Reference isobaric specific heat capacity
  const Real & _Cp;
};
