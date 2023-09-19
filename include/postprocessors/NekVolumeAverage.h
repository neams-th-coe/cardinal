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

#include "NekVolumeIntegral.h"

/**
 * Compute the average of a specified quantity over the volume of
 * the nekRS mesh, \f$\frac{\int_\Omega f d\Omega}{\int_\Omega d\Omega}\f$,
 * where \f$f\f$ is the field to be integrated, and \f$\Omega\f$ is the volume.
 *
 * Note that this calculation is done directly on the mesh that nekRS solves on,
 * _not_ the mesh created for solution transfer in NekRSMesh.
 */
class NekVolumeAverage : public NekVolumeIntegral
{
public:
  static InputParameters validParams();

  NekVolumeAverage(const InputParameters & parameters);

  virtual Real getValue() const override;
};
