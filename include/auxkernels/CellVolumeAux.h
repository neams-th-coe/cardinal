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

#include "OpenMCAuxKernel.h"

/**
 * Auxkernel to display the OpenMC cell mapped volumes
 */
class CellVolumeAux : public OpenMCAuxKernel
{
public:
  CellVolumeAux(const InputParameters & parameters);

  static InputParameters validParams();

protected:
  virtual Real computeValue();

  /// Interpretation of which volume to display on the Mesh
  const MooseEnum _volume_type;
};
