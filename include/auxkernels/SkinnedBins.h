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
#include "MoabSkinner.h"

/**
 * Auxkernel to display the mapping of [Mesh] elements to the spatial
 * bins created by a mesh skinner which skins by subdomain and arbitrary
 * user-specified scalar fields (e.g. temperature, density).
 */
class SkinnedBins : public AuxKernel
{
public:
  SkinnedBins(const InputParameters & parameters);

  static InputParameters validParams();

protected:
  virtual Real computeValue();

  /// Skinner object to be queried
  const MoabSkinner * _skinner;

  /**
   * What skinning bins to display; either 'all', 'subdomain', or the name
   * of one of the skinner's binned fields.
   */
  const std::string _skin_by;
};
