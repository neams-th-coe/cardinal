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

#include "ModelModifiersBase.h"

/**
 * Modifies the material filling an OpenMC cell prior to execution
 */
class OpenMCCellMaterialFill : public ModelModifiersBase
{
public:
  static InputParameters validParams();

  OpenMCCellMaterialFill(const InputParameters & parameters);

  virtual void modifyOpenMCModel() const override;

protected:
  /// Cell to be modified
  const int32_t & _cell_id;

  /// Index of the cell to be modified
  int32_t _cell_index;

  /// Materials to be filled into the cell instances
  std::vector<int32_t> _material_indices;
};
