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
  params.addParam<bool>("create_filter", false, "Whether to create the tally if it doesn't exist");
  params.addRequiredParam<int32_t>("filter_id", "The ID of the filter to modify");
  params.addParam<std::string>("filter_type", "", "The type of filter to create");
  params.addRequiredParam<std::vector<std::string>>("bins", "The bins to modify in the filter");
  params.declareControllable("bins");
  params.addClassDescription("A UserObject for creating and managing OpenMC domain tally filters");
  return params;
}

OpenMCDomainFilterEditor::OpenMCDomainFilterEditor(const InputParameters & parameters)
  : OpenMCUserObject(parameters),
    _filter_id(getParam<int32_t>("filter_id")),
    _filter_type(getParam<std::string>("filter_type"))
{}

void
OpenMCDomainFilterEditor::initialize()
{
  bool create_filter = getParam<bool>("create_filter");
  bool filter_exists = this->filter_exists();

  // check upon construction that the filter exists if we are not creating it
  if (!create_filter && !filter_exists)
  {
    mooseError(long_name() + ": Filter " + std::to_string(_filter_id) +
               " does not exist in the OpenMC XML model and create_filter is false");
  }

  // if create_filter is set to true, but the filter already exists, display a warning
  if (create_filter && filter_exists)
  {
    mooseWarning(long_name() + ": Filter " + std::to_string(_filter_id) +
                 " already exists in the OpenMC XML model");
  }

  // if this UO is to create the filter, but no filter type was specified, report an error
  if (create_filter && !filter_exists && _filter_type.empty())
  {
    mooseError(long_name() + ": filter_type must be specified when create_filter = true");
  }

  // if the filter exists and the filter type was not specified, accept the existing filter type
  if (filter_exists && _filter_type.empty())
  {
    // if an existing filter is being used, ensure that the filter type (if specified)
    // matches the existing filter type
    int32_t filter_index = openmc::model::filter_map.at(_filter_id);
    _filter_type = openmc::model::tally_filters[filter_index]->type_str();
    mooseWarning(long_name() + ": Filter " + std::to_string(_filter_id) +
                 " already exists in the OpenMC model. Using existing filter type: " +
                 _filter_type);
  }

  // at this point the filter type should be set, check that it is valid
  if (_allowed_types.count(_filter_type) == 0) {
    std::string msg =
    long_name() + ": Invalid filter type: " + _filter_type + ". Allowed types are: ";
    for (const auto & type : _allowed_types)
      msg += "\"" + type + "\"";
    mooseError(msg);
  }

  // if the filter doesn't exist at this point and no other errors have been raised,
  // create the filter
  if (!filter_exists)
  {
    openmc_problem()->_console << long_name() << ": Creating Filter " << _filter_id << std::endl;
    openmc::Filter::create(_filter_type, _filter_id);
  }

  // at this point the filter exists and the filter type is set, check that the type is valid
  check_filter_type_match();
}

void
OpenMCDomainFilterEditor::check_filter_type_match() const
{
  // check this UO's filter type against the one in the OpenMC model
  std::string existing_type = openmc::model::tally_filters[filter_index()]->type_str();
  if (existing_type != _filter_type)
  {
    mooseError(long_name() + ": An existing filter " + std::to_string(_filter_id) + " is of type " +
               existing_type + " and cannot be changed to type " + _filter_type);
  }
}

bool
OpenMCDomainFilterEditor::filter_exists() const
{
  return openmc::model::filter_map.find(_filter_id) != openmc::model::filter_map.end();
}

int32_t
OpenMCDomainFilterEditor::filter_index() const
{
  if (!filter_exists())
    mooseError(long_name() + ": Filter " + std::to_string(_filter_id) + " does not exist");
  return openmc::model::filter_map.at(_filter_id);
}

void
OpenMCDomainFilterEditor::first_execution()
{
  if (!_first_execution) return;
  initialize();
  _first_execution = false;
}

void
OpenMCDomainFilterEditor::execute()
{
  first_execution();

  openmc::Filter * filter = openmc::model::tally_filters[filter_index()].get();

  // TODO: update if webcontrols starts to support integral types
  std::vector<int32_t> ids, bins;
  for (auto bin_id : getParam<std::vector<std::string>>("bins"))
  {
    ids.push_back(std::stoi(bin_id));
  }

  if (_filter_type == "cell")
  {
    openmc::CellFilter * cell_filter = dynamic_cast<openmc::CellFilter *>(filter);
    if (!cell_filter)
      mooseError(long_name() + ": Filter " + std::to_string(_filter_id) + " is not a cell filter");
    for (auto id : ids)
      bins.push_back(openmc::model::cell_map.at(id));
    cell_filter->set_cells(bins);
  }
  else if (_filter_type == "material")
  {
    openmc::MaterialFilter * material_filter = dynamic_cast<openmc::MaterialFilter *>(filter);
    if (!material_filter)
      mooseError(long_name() + ": Filter " + std::to_string(_filter_id) +
                 " is not a material filter");
    for (auto id : ids)
      bins.push_back(openmc::model::material_map.at(id));
    material_filter->set_materials(bins);
  }
  else if (_filter_type == "universe")
  {
    openmc::UniverseFilter * universe_filter = dynamic_cast<openmc::UniverseFilter *>(filter);
    if (!universe_filter)
      mooseError(long_name() + ": Filter " + std::to_string(_filter_id) +
                 " is not a universe filter");
    for (auto id : ids)
      bins.push_back(openmc::model::universe_map.at(id));
    universe_filter->set_universes(bins);
  }
  else if (_filter_type == "mesh")
  {
    openmc::MeshFilter * mesh_filter = dynamic_cast<openmc::MeshFilter *>(filter);
    if (bins.size() != 1)
      mooseError(long_name() + ": Mesh filter must have exactly one bin");
    if (!mesh_filter)
      mooseError(long_name() + ": Filter " + std::to_string(_filter_id) + " is not a mesh filter");
    for (auto id : ids)
      bins.push_back(openmc::model::mesh_map.at(id));
    mesh_filter->set_mesh(bins[0]);
  }
}

#endif
