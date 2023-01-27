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
#include "MooseEnum.h"
#include "MoabSkinner.h"

/**
 * Auxkernel to display the mapping of [Mesh] elements to the spatial
 * bins created by a mesh skinner which skins by subdomain, temperature,
 * and density.
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
   * What skinning bins to display; this allows you to select just a single
   * "dimension" of the skinning to explore more thoroughly.
   */
  const MooseEnum _skin_by;
};
