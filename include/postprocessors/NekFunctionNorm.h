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

#include "NekPostprocessor.h"
#include "NekFieldInterface.h"
#include "CardinalEnums.h"
#include "Function.h"

/**
 * Compute the L$^N$ norm of a NekRS solution field relative to a function,
 * integrated over the NekRS mesh.
 *
 * Note that this calculation is done directly on the mesh that NekRS solves on,
 * _not_ the mesh created for solution transfer in NekRSMesh.
 */
class NekFunctionNorm : public NekPostprocessor, public NekFieldInterface
{
public:
  static InputParameters validParams();

  NekFunctionNorm(const InputParameters & parameters);

  virtual Real getValue() const override;

protected:
  /// Function to use for computing the norm
  const Function & _function;

  /// Order of the norm
  const unsigned int & _N;
};
