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

#include "NekFieldPostprocessor.h"

/**
 * Compute the integral of a specified quantity over the scalar
 * nekRS mesh, \f$\int_\Omega f d\Omega\f$, where \f$f\f$ is the
 * field to be integrated, and \f$\Omega\f$ is the volume.
 *
 * Note that this calculation is done directly on the mesh that nekRS solves on,
 * _not_ the mesh created for solution transfer in NekRSMesh.
 */
class NekVolumeIntegral : public NekFieldPostprocessor
{
public:
  static InputParameters validParams();

  NekVolumeIntegral(const InputParameters & parameters);

  virtual Real getValue() override;

  /**
   * Integrate the field over the specified mesh
   * @param[in] mesh mesh
   * @return field integrated over the mesh
   */
  Real getIntegralOnMesh(const nek_mesh::NekMeshEnum & mesh);

protected:
  /// Volume by which to normalize
  Real _volume;
};
