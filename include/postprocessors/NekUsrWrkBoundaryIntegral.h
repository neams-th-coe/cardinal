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
 * Compute the integral of an entry in the nrs->usrwrk array over the nekRS mesh,
 * \f$\int_\Gamma q d\Gamma\f$, where \f$q\f$ is the entry in the nrs->usrwrk array
 * and \f$\Gamma\f$ is the boundary.
 *
 * Note that this calculation is done directly on the mesh that nekRS solves on,
 * _not_ the mesh created for solution transfer in NekRSMesh.
 */
class NekUsrWrkBoundaryIntegral : public NekSidePostprocessor
{
public:
  static InputParameters validParams();

  NekUsrWrkBoundaryIntegral(const InputParameters & parameters);

  virtual Real getValue() const override;

protected:
  /// Slot in usrwrk array to integrate
  const unsigned int & _usrwrk_slot;
};
