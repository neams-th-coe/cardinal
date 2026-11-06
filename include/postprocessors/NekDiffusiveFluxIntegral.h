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
 * Compute the integral of the diffusive flux over a boundary
 * in the NekRS mesh, \f$\int_\Gamma -k\nabla T\cdot\hat{n} d\Gamma\f$, where \f$T\f$ is the
 * temperature, \f$k\f$ is the thermal conductivity,
 * \f$\hat{n}\f$ is the unit normal, and \f$\Gamma\f$ is the boundary. A generic
 * scalar can be provided, in which case the interpretation of this object becomes
 * e.g. heat flux, mass flux, etc.
 *
 * Note that this calculation is done directly on the mesh that nekRS solves on,
 * _not_ the mesh created for solution transfer in NekRSMesh.
 */
class NekDiffusiveFluxIntegral : public NekSidePostprocessor
{
public:
  static InputParameters validParams();

  NekDiffusiveFluxIntegral(const InputParameters & parameters);

  virtual Real getValue() const override;

protected:
  /// Name of the scalar
  const std::string _scalar_name;

  /// Index for the scalar to compute the diffusive flux for
  const unsigned int _scalar_id;
};
