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

#include "CriticalitySearchBase.h"

/**
 * Perform a criticality search based on a material
 */
class OpenMCMaterialSearch : public CriticalitySearchBase
{
public:
  static InputParameters validParams();

  OpenMCMaterialSearch(const InputParameters & parameters);

protected:
  /// Material to be modified
  const int32_t & _material_id;

  /// Material index corresponding to the ID
  int32_t _material_index;
};
