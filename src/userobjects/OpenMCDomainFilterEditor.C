/********************************************************************/
/*                  SOFTWARE COPYRIGHT NOTIFICATION                 */
/*                             Cardinal                             */
/*                                                                  */
/*                  (c) 2024 UChicago Argonne, LLC                  */
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

#include "OpenMCDomainFilterEditor.h"
#include "OpenMCProblemBase.h"
#include "openmc/tallies/tally.h"
#include "openmc/tallies/filter.h"
#include "openmc/tallies/filter_cell.h"
#include "openmc/tallies/filter_material.h"
#include "openmc/tallies/filter_universe.h"
#include "openmc/tallies/filter_mesh.h"

registerMooseObject("CardinalApp", OpenMCDomainFilterEditor);

InputParameters
OpenMCDomainFilterEditor::validParams()
{
  InputParameters params = OpenMCUserObject::validParams();
  params.addParam<bool>("create_filter", false, "Whether to create the filter if it doesn't exist");
  params.addRequiredParam<int32_t>("filter_id", "The ID of the filter to modify");
  params.addRequiredParam<MooseEnum>(
      "filter_type", getFilterTypeEnum(), "The type of filter to create");
  params.addRequiredParam<std::vector<std::string>>("bins", "The bins to modify in the filter");
  params.declareControllable("bins");
  params.addClassDescription("A UserObject for creating and managing OpenMC domain tally filters");
  return params;
}

OpenMCDomainFilterEditor::OpenMCDomainFilterEditor(const InputParameters & parameters)
  : OpenMCUserObject(parameters),
    _create_filter(getParam<bool>("create_filter")),
    _filter_id(getParam<int32_t>("filter_id")),
    _filter_type(getParam<MooseEnum>("filter_type").getEnum<OpenMCFilterType>())
{
  this->initialize();
}

void
OpenMCDomainFilterEditor::initialize()
{
  bool filter_exists = this->filterExists();

  // if create_filter is set to true, but the filter already exists, display a warning
  if (_create_filter && filter_exists)
  {
    mooseWarning(longName() + ": Filter " + std::to_string(_filter_id) +
                 " already exists in the OpenMC XML model");
  }

  if (!_create_filter && !filter_exists)
    paramError("filter_id",
               "Filter " + std::to_string(_filter_id) +
                   " does not exist and create_filter is false");

  // if the filter doesn't exist at this point and no other errors have been raised,
  // create the filter
  if (!filter_exists)
    openmc::Filter::create(filterTypeEnumToString(_filter_type), _filter_id);

  // at this point the filter exists and the filter type is set, check that the type is valid
  checkFilterTypeMatch();
}

void
OpenMCDomainFilterEditor::checkFilterTypeMatch() const
{
  // check this UO's filter type against the one in the OpenMC model
  std::string existing_type_str = openmc::model::tally_filters[filterIndex()]->type_str();
  OpenMCFilterType existing_type = stringToFilterTypeEnum(existing_type_str);
  _console << "CHECKING FILTER TYPE..." << std::endl;

  if (existing_type != _filter_type)
    paramError("filter_id",
               "An existing filter, Filter " + std::to_string(_filter_id) + ", is of type \"" +
                   existing_type_str + "\" and cannot be changed to type \"" +
                   filterTypeEnumToString(_filter_type) + "\"");
}

bool
OpenMCDomainFilterEditor::filterExists() const
{
  return openmc::model::filter_map.find(_filter_id) != openmc::model::filter_map.end();
}

int32_t
OpenMCDomainFilterEditor::filterIndex() const
{
  return openmc::model::filter_map.at(_filter_id);
}

void
OpenMCDomainFilterEditor::execute()
{
  openmc::Filter * filter = openmc::model::tally_filters[filterIndex()].get();

  // TODO: update if webcontrols starts to support integral types
  std::vector<int32_t> ids, bins;
  for (auto bin_id : getParam<std::vector<std::string>>("bins"))
  {
    ids.push_back(std::stoi(bin_id));
  }

  if (_filter_type == OpenMCFilterType::cell)
  {
    openmc::CellFilter * cell_filter = dynamic_cast<openmc::CellFilter *>(filter);
    if (!cell_filter)
      mooseError(longName() + ": Filter " + std::to_string(_filter_id) + " is not a cell filter");
    for (auto id : ids)
      bins.push_back(openmc::model::cell_map.at(id));
    cell_filter->set_cells(bins);
  }
  else if (_filter_type == OpenMCFilterType::material)
  {
    openmc::MaterialFilter * material_filter = dynamic_cast<openmc::MaterialFilter *>(filter);
    if (!material_filter)
      mooseError(longName() + ": Filter " + std::to_string(_filter_id) +
                 " is not a material filter");
    for (auto id : ids)
      bins.push_back(openmc::model::material_map.at(id));
    material_filter->set_materials(bins);
  }
  else if (_filter_type == OpenMCFilterType::universe)
  {
    openmc::UniverseFilter * universe_filter = dynamic_cast<openmc::UniverseFilter *>(filter);
    if (!universe_filter)
      mooseError(longName() + ": Filter " + std::to_string(_filter_id) +
                 " is not a universe filter");
    for (auto id : ids)
      bins.push_back(openmc::model::universe_map.at(id));
    universe_filter->set_universes(bins);
  }
  else if (_filter_type == OpenMCFilterType::mesh)
  {
    openmc::MeshFilter * mesh_filter = dynamic_cast<openmc::MeshFilter *>(filter);
    if (bins.size() != 1)
      mooseError(longName() + ": Mesh filter must have exactly one bin");
    if (!mesh_filter)
      mooseError(longName() + ": Filter " + std::to_string(_filter_id) + " is not a mesh filter");
    for (auto id : ids)
      bins.push_back(openmc::model::mesh_map.at(id));
    mesh_filter->set_mesh(bins[0]);
  }
}

std::string
OpenMCDomainFilterEditor::filterTypeEnumToString(OpenMCFilterType t) const
{
  if (t == OpenMCFilterType::cell)
    return "cell";
  else if (t == OpenMCFilterType::material)
    return "material";
  else if (t == OpenMCFilterType::universe)
    return "universe";
  else if (t == OpenMCFilterType::mesh)
    return "mesh";
  else if (t == OpenMCFilterType::none)
    return "none";
  else
    mooseError("Invalid filter type");
}

OpenMCFilterType
OpenMCDomainFilterEditor::stringToFilterTypeEnum(const std::string & s) const
{
  if (s == "cell")
    return OpenMCFilterType::cell;
  else if (s == "material")
    return OpenMCFilterType::material;
  else if (s == "universe")
    return OpenMCFilterType::universe;
  else if (s == "mesh")
    return OpenMCFilterType::mesh;
  else
    mooseError("Invalid filter type");
}

void
OpenMCDomainFilterEditor::duplicateFilterError(const int32_t & id) const
{
  paramError("filter_id",
             "Duplicate filter ID (" + std::to_string(id) +
                 ") found across multiple OpenMCDomainFilterEditors");
}

#endif
