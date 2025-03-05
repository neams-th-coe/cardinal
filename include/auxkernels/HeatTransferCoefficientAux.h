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

#include "AuxKernel.h"

/**
 * Compute a heat transfer coefficient as q / (T_w - T_inf) with each
 * of these three quantities obtained from a user object.
 */
class HeatTransferCoefficientAux : public AuxKernel
{
public:
  HeatTransferCoefficientAux(const InputParameters & parameters);

  static InputParameters validParams();

protected:
  virtual Real computeValue();

  /// User object containing heat flux
  const UserObject & _heat_flux;

  /// User object containing wall temperature
  const UserObject & _wall_T;

  /// User object containing bulk temperature
  const UserObject & _bulk_T;
};
