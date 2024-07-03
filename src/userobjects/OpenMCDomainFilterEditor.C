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
  InputParameters params = GeneralUserObject::validParams();
  params.addParam<bool>("create_filter", false, "Whether to create the tally if it doesn't exist");
  params.addRequiredParam<int32_t>("filter_id", "The ID of the filter to modify");
  params.addRequiredParam<std::string>("filter_type", "The type of filter to create");
  params.addRequiredParam<std::vector<std::string>>("bins", "The bins to modify in the filter");
  params.declareControllable("bins");
  params.addClassDescription("A UserObject for creating and managing OpenMC tallies");
  return params;
}

OpenMCDomainFilterEditor::OpenMCDomainFilterEditor(const InputParameters & parameters)
  : GeneralUserObject(parameters), _filter_id(getParam<int32_t>("filter_id")), _filter_type(getParam<std::string>("filter_type"))
{
  const OpenMCProblemBase * openmc_problem = dynamic_cast<const OpenMCProblemBase *>(&_fe_problem);
  if (!openmc_problem)
  {
    std::string extra_help = _fe_problem.type() == "FEProblem" ? " (the default)" : "";
    mooseError("This user object can only be used with wrapped OpenMC cases! "
               "You need to change the\nproblem type from '" +
               _fe_problem.type() + "'" + extra_help + " to OpenMCCellAverageProblem.");
  }


  bool create_filter = getParam<bool>("create_filter");
  bool filter_exists = openmc::model::filter_map.find(_filter_id) != openmc::model::filter_map.end();

  if (create_filter) {
    if (filter_exists)
    {
      mooseWarning(long_name() + ": Filter " + std::to_string(_filter_id) + " already exists in OpenMC model");
    }
    else
    {
      openmc_problem->_console << long_name() << ": Creating Filter " << _filter_id << std::endl;
      openmc::Filter::create(_filter_type, _filter_id);
    }
  } else {
    if (!filter_exists)
    {
      mooseError(long_name() + ": Filter " + std::to_string(_filter_id) + " does not exist in OpenMC model");
    } else {
      check_existing_filter_type();
    }
  }

  if (_allowed_types.count(_filter_type) == 0)
    bad_filter_error();
}

void
OpenMCDomainFilterEditor::check_existing_filter_type() const {
  int32_t filter_index = openmc::model::filter_map.at(_filter_id);
  std::string existing_type = openmc::model::tally_filters[filter_index]->type_str();
  if (existing_type != _filter_type)
  {
    mooseError(long_name() + ": An existing filter " + std::to_string(_filter_id) + " is of type " + existing_type +
               " and cannot be changed to type " + _filter_type);
  }
}

void
OpenMCDomainFilterEditor::bad_filter_error() const {
  std::string msg = long_name() + ": Invalid filter type: " + _filter_type;
  msg += ". Allowed types are: ";
  for (const auto & type : _allowed_types)
    msg += "\"" + type + "\"";
  mooseError(msg);
}

void
OpenMCDomainFilterEditor::execute()
{

  int32_t filter_index = openmc::model::filter_map.at(_filter_id);
  openmc::Filter * filter = openmc::model::tally_filters[filter_index].get();

  // TODO: update if webcontrols starts to support integral types
  std::vector<int32_t> ids, bins;
  for (auto bin_id : getParam<std::vector<std::string>>("bins"))
  {
    ids.push_back(std::stoi(bin_id));
  }

  if (_filter_type == "cell") {
    openmc::CellFilter * cell_filter = dynamic_cast<openmc::CellFilter*>(filter);
    if (!cell_filter)
      mooseError(long_name() + ": Filter " + std::to_string(_filter_id) + " is not a cell filter");
    for (auto id : ids) bins.push_back(openmc::model::cell_map.at(id));
    cell_filter->set_cells(bins);
  } else if (_filter_type == "material") {
    openmc::MaterialFilter * material_filter = dynamic_cast<openmc::MaterialFilter*>(filter);
    if (!material_filter)
      mooseError(long_name() + ": Filter " + std::to_string(_filter_id) + " is not a material filter");
    for (auto id : ids) bins.push_back(openmc::model::material_map.at(id));
    material_filter->set_materials(bins);
  } else if (_filter_type == "universe") {
    openmc::UniverseFilter * universe_filter = dynamic_cast<openmc::UniverseFilter*>(filter);
    if (!universe_filter)
      mooseError(long_name() + ": Filter " + std::to_string(_filter_id) + " is not a universe filter");
    for (auto id : ids) bins.push_back(openmc::model::universe_map.at(id));
    universe_filter->set_universes(bins);
  } else if (_filter_type == "mesh") {
    openmc::MeshFilter * mesh_filter = dynamic_cast<openmc::MeshFilter*>(filter);
    if (bins.size() != 1)
      mooseError(long_name() + ": Mesh filter must have exactly one bin");
    if (!mesh_filter)
      mooseError(long_name() + ": Filter " + std::to_string(_filter_id) + " is not a mesh filter");
    for (auto id : ids) bins.push_back(openmc::model::mesh_map.at(id));
    mesh_filter->set_mesh(bins[0]);
  }
}

#endif
