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

#include "NekSidePostprocessor.h"

/**
 * Compute the Reynolds number as
 * \f$\frac{\dot{m}L}{A\mu}\f$, where $\dot{m}$ is the mass flowrate,
 * \f$L\f$ is a characteristic length, \f$A\f$ is the area, and \f$\mu\f$
 * is the dynamic viscosity. The 'boundary' is used to specify which
 * boundary of NekRS's mesh to use for computing the mass flowrate and area.
 *
 * If NekRS is run in dimensional form, then the characteristic length
 * must be provided manually - otherwise, it is inferred from the
 * Reynolds number given in the input file.
 */
class ReynoldsNumber : public NekSidePostprocessor
{
public:
  static InputParameters validParams();

  ReynoldsNumber(const InputParameters & parameters);

  virtual Real getValue() const override;

protected:
  /// Characteristic length
  const Real * _L_ref;
};
