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
  params.addParam<std::vector<int32_t>>(
      "material_ids",
      "IDs of the materials to fill into the cell; the length of this array can be one material ID "
      "that will be applied to all instances. Otherwise, it must match the number "
      "of instances of the cell");
  params.addParam<std::string>(
      "material_ids_file",
      "Filename containing material ids to assign to the cell instances. Must be an h5 file.");
  params.addClassDescription("Modifies the cell fill within an OpenMC cell");
  return params;
}

OpenMCCellMaterialFill::OpenMCCellMaterialFill(const InputParameters & parameters)
  : ModelModifiersBase(parameters), _cell_id(getParam<int32_t>("cell_id"))
{
  // check that only one of material_ids and material_ids_file is specified
  const bool material_ids_valid = isParamValid("material_ids");
  const bool material_ids_file_valid = isParamValid("material_ids_file");
  if (!material_ids_valid && !material_ids_file_valid)
    mooseError("One of 'material_ids' or 'material_ids_file' must be specified, but neither have "
               "been provided.");
  if (material_ids_valid && material_ids_file_valid)
    mooseError("Both of 'material_ids' and 'material_ids_file' were be specified, but only one can "
               "be provided.");

  // get the cell requested
  _cell_index = -1;
  int err = openmc_get_cell_index(_cell_id, &_cell_index);
  if (err)
    paramError("cell_id",
               "Cell ID " + std::to_string(_cell_id) + " was not found in the OpenMC model!");

  auto n_cell_instances = openmc::model::cells[_cell_index]->n_instances();
  if (n_cell_instances == 0)
    paramError("cell_id",
               "Cell ID " + std::to_string(_cell_id) +
                   " has zero instances so its material fill cannot be modified. This cell is "
                   "probably corresponding to a cell in a lattice.outer universe. If you want to "
                   "change the material here, you will need to either (i) widen your lattice to "
                   "have universes covering all of the space you want to change the material or "
                   "(ii) represent this region of space as a conventional cell.\n\nFor more "
                   "information, see: https://github.com/openmc-dev/openmc/issues/551.");

  std::vector<int32_t> material_ids;
  // check that the length of the materials matches the number of instances
  if (material_ids_valid)
    material_ids = getParam<std::vector<int32_t>>("material_ids");
  else
    extractMaterialIdsFromFile(getParam<std::string>("material_ids_file"), material_ids);

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
    _material_indices.assign(n_cell_instances, material_index);
  }
}

void
OpenMCCellMaterialFill::extractMaterialIdsFromFile(std::string filename,
                                                   std::vector<int32_t> & material_ids)
{
  // confirm that filename specified exists
  if (!MooseUtils::checkFileReadable(filename))
  {
    paramError("material_ids_file", "The filename specified could not be found.");
  }

  // extract the material_fill_ids attribute from the file
  hid_t file = openmc::file_open(filename, 'r');
  openmc::read_dataset(file, "material_fill_ids", material_ids);
  openmc::file_close(file);
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
