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

  /**
   * Read data from hdf5 file to assign to _material_indices
   * @param[in] filename filename
   * @param[out] material_ids passed by reference to assign
   */
  void extractMaterialIdsFromFile(std::string filename, std::vector<int32_t> & material_ids);

  virtual void modifyOpenMCModel() const override;

  // Accessor for the cell ID asociated with the OpenMCCellMaterialFill ModelModifier
  const int32_t & getCellId() const { return _cell_id; }

  // Accessor for the cell index asociated with the OpenMCCellMaterialFill ModelModifier
  const int32_t & getCellIndex() const { return _cell_index; }

  // Accessor for the material indices corresponding the OpenMCCellMaterialFill ModelModifier that
  // will be applied to the cell
  const std::vector<int32_t> & getMaterialIndices() const { return _material_indices; }

protected:
  /// Cell to be modified
  const int32_t & _cell_id;

  /// Index of the cell to be modified
  int32_t _cell_index;

  /// Materials to be filled into the cell instances
  std::vector<int32_t> _material_indices;

  /// Name of a file containing material data to be assigned to _material_indices
  std::string _material_ids_file;
};
