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

#include "NekSideIntegral.h"

/**
 * Compute the average of a specified quantity over the boundaries
 * in the nekRS mesh, \f$\frac{\int_\Gamma f d\Gamma}{\int_\Gamma d\Gamma}\f$,
 * where \f$f\f$ is the field to be integrated, and \f$\Gamma\f$ is the boundary.
 *
 * Note that this calculation is done directly on the mesh that nekRS solves on,
 * _not_ the mesh created for solution transfer in NekRSMesh.
 */
class NekSideAverage : public NekSideIntegral
{
public:
  static InputParameters validParams();

  NekSideAverage(const InputParameters & parameters);

  virtual Real getValue() const override;
};
