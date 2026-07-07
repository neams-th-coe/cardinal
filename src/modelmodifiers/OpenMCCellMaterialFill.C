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

#ifdef ENABLE_OPENMC_COUPLING

#include "OpenMCCellMaterialFill.h"
#include "UserErrorChecking.h"
#include "openmc/cell.h"

registerMooseObject("CardinalApp", OpenMCCellMaterialFill);

InputParameters
OpenMCCellMaterialFill::validParams()
{
  auto params = ModelModifiersBase::validParams();
  params.addRequiredParam<int32_t>("cell_id", "Cell ID to modify");
  params.addRequiredParam<std::vector<int32_t>>(
      "material_ids",
      "IDs of the materials to fill into the cell; the length of this array must match the number "
      "of instances of the cell");
  params.addClassDescription("Modifies the cell fill within an OpenMC cell");
  return params;
}

OpenMCCellMaterialFill::OpenMCCellMaterialFill(const InputParameters & parameters)
  : ModelModifiersBase(parameters), _cell_id(getParam<int32_t>("cell_id"))
{
  // get the cell requested
  _cell_index = -1;
  int err = openmc_get_cell_index(_cell_id, &_cell_index);
  if (err)
    paramError("cell_id",
               "Cell ID " + std::to_string(_cell_id) + " was not found in the OpenMC model!");

  // check that the length of the materials matches the number of instances
  const auto & material_ids = getParam<std::vector<int32_t>>("material_ids");
  auto n_cell_instances = openmc::model::cells[_cell_index]->n_instances();
  auto n_mats = std::size(material_ids);
  if (n_mats != 1 && (n_cell_instances != n_mats))
    paramError("material_ids",
               "'material_ids' (length " + std::to_string(n_mats) +
                   ") must either be of length 1 or have a length equal to the number of instances "
                   "of cell " +
                   std::to_string(_cell_id) + " (" + std::to_string(n_cell_instances) + ")");

  // check that each provided material exists in the model
  int32_t material_index = -1;
  for (const auto & m : material_ids)
  {
    err = openmc_get_material_index(m, &material_index);
    if (err)
      paramError("material_ids",
                 "Material with ID " + std::to_string(m) + " was not found in the OpenMC model!");
    _material_indices.push_back(material_index);
  }

  if (n_mats == 1)
  {
    // make _material_indices equal in length to the number of cell instances
    for (int i = 1; i < n_mats; ++i)
      _material_indices.push_back(material_index);
  }
}

void
OpenMCCellMaterialFill::modifyOpenMCModel() const
{
  const auto n_mats = std::size(_material_indices);
  std::string m = n_mats > 1 ? "materials" : "material";
  _console << "Modifying " << m << " in cell " << _cell_id << "..." << std::endl;

  int err = openmc_cell_set_fill(_cell_index, 0, n_mats, _material_indices.data());
  catchOpenMCError(err, "attempting to set a material fill in cell " + std::to_string(_cell_id));
}

#endif
