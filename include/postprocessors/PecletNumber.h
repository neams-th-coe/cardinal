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

#include "ReynoldsNumber.h"

/**
 * Compute the Peclet number as
 * \f$Re\ Pr\f$, where $Re$ is the Reynolds number and \f$Pr\f$ is the
 * Prandtl number. The 'boundary' is used to specify which
 * boundary of NekRS's mesh to use for computing the mass flowrate and area.
 *
 * If NekRS is run in dimensional form, then the characteristic length
 * must be provided manually - otherwise, it is inferred from the
 * Reynolds number given in the input file.
 */
class PecletNumber : public ReynoldsNumber
{
public:
  static InputParameters validParams();

  PecletNumber(const InputParameters & parameters);

  virtual Real getValue() const override;
};
