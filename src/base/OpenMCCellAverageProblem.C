#include "AuxiliarySystem.h"
#include "DelimitedFileReader.h"
#include "TimedPrint.h"
#include "MooseUtils.h"

#include "mpi.h"
#include "OpenMCCellAverageProblem.h"
#include "openmc/capi.h"
#include "openmc/cell.h"
#include "openmc/constants.h"
#include "openmc/error.h"
#include "openmc/material.h"
#include "openmc/particle.h"
#include "openmc/geometry.h"
#include "openmc/message_passing.h"
#include "openmc/settings.h"
#include "openmc/summary.h"
#include "xtensor/xarray.hpp"
#include "xtensor/xview.hpp"

registerMooseObject("CardinalApp", OpenMCCellAverageProblem);

bool OpenMCCellAverageProblem::_first_transfer = true;

template<>
InputParameters
validParams<OpenMCCellAverageProblem>()
{
  InputParameters params = validParams<ExternalProblem>();
  params.addRequiredRangeCheckedParam<Real>("power", "power >= 0.0",
    "Power (Watts) to normalize the OpenMC tallies; this is the power "
    "produced by the entire OpenMC problem.");
  params.addParam<std::vector<SubdomainID>>("fluid_blocks",
    "Subdomain ID(s) corresponding to the fluid phase, "
    "for which both density and temperature will be sent to OpenMC");
  params.addParam<std::vector<SubdomainID>>("solid_blocks",
    "Subdomain ID(s) corresponding to the solid phase, "
    "for which temperature will be sent to OpenMC");
  params.addParam<std::vector<SubdomainID>>("tally_blocks",
    "Subdomain ID(s) for which to add tallies in the OpenMC model; "
    "only used with cell tallies");
  params.addParam<bool>("check_tally_sum",
    "Whether to check consistency between the cell-wise kappa fission tallies with a global tally");
  params.addParam<bool>("check_zero_tallies", true,
    "Whether to throw an error if any tallies from OpenMC evaluate to zero; "
    "this can be helpful in reducing the number of tallies if you inadvertently add tallies "
    "to a non-fissile region, or for catching geomtery setup errors");
  params.addParam<bool>("verbose", false, "Whether to print diagnostic information");
  params.addParam<bool>("skip_first_incoming_transfer", false,
    "Whether to skip the very first density and temperature transfer into OpenMC; "
    "this can be used to allow whatever initial condition is set in OpenMC's XML "
    "files to be used in OpenMC's run the first time OpenMC is run");
  params.addRangeCheckedParam<Real>("scaling", 1.0, "scaling > 0.0",
    "Scaling factor to apply to mesh to get to units of centimeters that OpenMC expects; "
    "setting 'scaling = 100.0', for instance, indicates that the mesh is in units of meters");
  params.addParam<bool>("normalize_by_global_tally", true,
    "Whether to normalize by a global kappa-fission tally (true) or else by the sum "
    "of the local tally (false)");

  params.addParam<MooseEnum>("tally_filter", getTallyCellFilterEnum(),
    "Type of filter to apply to the cell tally, options: cell, cell_instance (default). "
    "You will generally always want to use the cell_instance option to allow cases with "
    "distributed cells, but then you are limited to only material fills");
  params.addRequiredParam<MooseEnum>("tally_type", getTallyTypeEnum(),
    "Type of tally to use in OpenMC, options: cell, mesh");
  params.addParam<std::string>("mesh_template", "Mesh tally template for OpenMC when using mesh tallies; "
    "at present, this mesh must exactly match the mesh used in the [Mesh] block because a one-to-one copy "
    "is used to get OpenMC's tally results on the [Mesh] in preparation for transfer to another App.");
  params.addParam<std::vector<Point>>("mesh_translations",
    "Coordinates to which each mesh template should be translated, if multiple unstructured meshes "
    "are desired.");
  params.addParam<std::vector<FileName>>("mesh_translations_file",
    "File providing the coordinates to which each mesh template should be translated, if multiple "
    "unstructured meshes are desired.");

  params.addParam<int>("solid_cell_level", "Coordinate level in OpenMC to stop at for identifying solid cells");
  params.addParam<int>("fluid_cell_level", "Coordinate level in OpenMC to stop at for identifying fluid cells");
  return params;
}

OpenMCCellAverageProblem::OpenMCCellAverageProblem(const InputParameters &params) :
  ExternalProblem(params),
  _serialized_solution(NumericVector<Number>::build(_communicator).release()),
  _tally_filter(getParam<MooseEnum>("tally_filter").getEnum<filter::CellFilterEnum>()),
  _tally_type(getParam<MooseEnum>("tally_type").getEnum<tally::TallyTypeEnum>()),
  _power(getParam<Real>("power")),
  _check_zero_tallies(getParam<bool>("check_zero_tallies")),
  _verbose(getParam<bool>("verbose")),
  _skip_first_incoming_transfer(getParam<bool>("skip_first_incoming_transfer")),
  _specified_scaling(params.isParamSetByUser("scaling")),
  _scaling(getParam<Real>("scaling")),
  _normalize_by_global(getParam<bool>("normalize_by_global_tally")),
  _check_tally_sum(isParamValid("check_tally_sum") ? getParam<bool>("check_tally_sum") : _normalize_by_global),
  _has_fluid_blocks(params.isParamSetByUser("fluid_blocks")),
  _has_solid_blocks(params.isParamSetByUser("solid_blocks")),
  _needs_global_tally(_check_tally_sum || _normalize_by_global),
  _single_coord_level(openmc::model::n_coord_levels == 1),
  _n_openmc_cells(openmc::model::cells.size()),
  _n_cell_digits(digits(_n_openmc_cells)),
  _using_default_tally_blocks(_tally_type == tally::cell && _single_coord_level && !isParamValid("tally_blocks"))
{
  if (openmc::settings::libmesh_comm)
    mooseWarning("libMesh communicator already set in OpenMC.");

  openmc::settings::libmesh_comm = &_mesh.comm();

  // for cases where OpenMC is the master app and we have two sub-apps that represent (1) fluid region,
  // and (2) solid region, we can save on one transfer if OpenMC computes the heat flux from a transferred
  // temperature (as opposed to the solid app sending both temperature and heat flux). Temperature is always
  // transferred. Because we need a material property to represent thermal conductivity, MOOSE's default
  // settings will force OpenMC to have materials on every block, when that's not actually needed. So
  // we can turn that check off.
  setMaterialCoverageCheck(false);

  switch (_tally_type)
  {
    case tally::cell:
    {
      std::vector<std::string> unused_pars = {"mesh_template", "mesh_translations", "mesh_translations_file"};

      for (const auto & s : unused_pars)
        if (params.isParamSetByUser(s))
          mooseWarning("The '" + s + "' parameter is unused when using cell tallies!");

      // tally_blocks is optional if the OpenMC geometry has a single coordinate level
      if (!_single_coord_level && !isParamValid("tally_blocks"))
        paramError("tally_blocks", "List of tally blocks must be specified for OpenMC geometries with "
          "more than one coordinate level");

      readTallyBlocks();

      // For single-level geometries, we take the default setting for tally_blocks to be all the
      // blocks in the MOOSE domain
      if (_using_default_tally_blocks)
        for (const auto & s : _mesh.meshSubdomains())
          _tally_blocks.insert(s);

      break;
    }
    case tally::mesh:
    {
      std::vector<std::string> unused_pars = {"tally_filter", "tally_blocks"};

      for (const auto & s : unused_pars)
        if (params.isParamSetByUser(s))
          mooseWarning("The '" + s + "' parameter is unused when using mesh tallies!");

      if (isParamValid("mesh_translations") && isParamValid("mesh_translations_file"))
        mooseError("Both 'mesh_translations' and 'mesh_translations_file' cannot be specified");

      if (!isParamValid("mesh_template"))
        paramError("mesh_template", "When using a mesh tally, a mesh template must be provided!");

       _mesh_template_filename = getParam<std::string>("mesh_template");

      if (_mesh_template_filename.empty())
        paramError("mesh_template", "When using a mesh tally, the mesh template cannot be empty!");

      fillMeshTranslations();

      break;
    }
    default:
      mooseError("Unhandled TallyTypeEnum in OpenMCCellAverageProblem!");
  }

  if (!isParamValid("fluid_blocks") && !isParamValid("solid_blocks"))
    mooseError("At least one of 'fluid_blocks' and 'solid_blocks' must be specified to "
      "establish the mapping from MOOSE to OpenMC.");

  readFluidBlocks();
  readSolidBlocks();

  // Make sure the same block ID doesn't appear in both the fluid and solid blocks,
  // or else we won't know how to send feedback into OpenMC.
  checkBlockOverlap();

  // get the coordinate level to find cells on for each phase, and warn if invalid or not used
  getCellLevel("fluid", _fluid_cell_level);
  getCellLevel("solid", _solid_cell_level);

  initializeElementToCellMapping();

  getMaterialFills();

  initializeTallies();

  checkMeshTemplateAndTranslations();
}

OpenMCCellAverageProblem::~OpenMCCellAverageProblem()
{
  openmc_finalize();
}

template <typename T>
void
OpenMCCellAverageProblem::checkEmptyVector(const std::vector<T> & vector, const std::string & name) const
{
  if (vector.empty())
    paramError(name, "Vector cannot be empty!");
}

void
OpenMCCellAverageProblem::fillMeshTranslations()
{
  if (isParamValid("mesh_translations"))
  {
    _mesh_translations = getParam<std::vector<Point>>("mesh_translations");
    checkEmptyVector(_mesh_translations, "mesh_translations");
  }
  else if (isParamValid("mesh_translations_file"))
  {
    std::vector<FileName> mesh_translations_file = getParam<std::vector<FileName>>("mesh_translations_file");
    checkEmptyVector(mesh_translations_file, "mesh_translations_file");

    for (const auto & f : mesh_translations_file)
    {
      MooseUtils::DelimitedFileReader file(f, &_communicator);
      file.setFormatFlag(MooseUtils::DelimitedFileReader::FormatFlag::ROWS);
      file.read();

      const std::vector<std::vector<double>> & data = file.getData();
      readMeshTranslations(data);
    }
  }
  else
    _mesh_translations = {Point(0.0, 0.0, 0.0)};
}

void
OpenMCCellAverageProblem::checkMeshTemplateAndTranslations()
{
  // we can do some rudimentary checking on the mesh template by comparing the centroid
  // coordinates compared to centroids in the [Mesh] (because right now, we just doing a simple
  // copy transfer that necessitates the meshes to have the same elements in the same order). In
  // other words, you might have two meshes that represent the same geometry, but if you created
  // the solid phase _first_ in Cubit for one mesh, but the fluid phase _first_ in Cubit for the
  // other mesh, even though the geometry is the same, the element ordering would be different.
  //
  // If the first two elements of each mesh translation match the [Mesh], we assume that the meshes
  // are the same (otherwise, print an error). We need to check two elements per mesh translation
  // because this ensures that both the position and angular rotation match.
  unsigned int offset = 0;
  for (unsigned int i = 0; i < _mesh_filters.size(); ++i)
  {
    const auto & filter = _mesh_filters[i];

    // just compare the first two elements
    for (unsigned int e = 0; e < 2; ++e)
    {
      auto elem_ptr = _mesh.queryElemPtr(offset + e);

      // if element is not on this part of the distributed mesh, skip it
      if (!elem_ptr)
        continue;

      auto pt = _mesh_template->centroid(e);
      Point centroid_template = {pt[0] , pt[1], pt[2]};

      // The translation applied in OpenMC isn't actually registered in the mesh itself;
      // it is always added on to the point, so we need to do the same here
      centroid_template += _mesh_translations[i];

      // because the mesh template and [Mesh] may be in different units, we need
      // to adjust the [Mesh] by the scaling factor before doing a comparison.
      Point centroid_mesh = elem_ptr->centroid() * _scaling;

      // if the centroids are the same except for a factor of 'scaling', then we can
      // guess that the mesh_template is probably not in units of centimeters
      if (_specified_scaling)
      {
        // if scaling was applied correctly, then each calculation of 'scaling' here should equal 1. Otherwise,
        // if they're all the same, then 'scaling_x' is probably the factor by which the mesh_template
        // needs to be multiplied, so we can print a helpful error message
        bool incorrect_scaling = true;
        for (unsigned int j = 0; j < DIMENSION; ++j)
        {
          Real scaling = centroid_mesh(j) / centroid_template(j);
          incorrect_scaling = incorrect_scaling && !MooseUtils::absoluteFuzzyEqual(scaling, 1.0);
        }

        if (incorrect_scaling)
          mooseError("The centroids of the 'mesh_template' (assumed to be in units of cm) differ from the "
            "centroids of the [Mesh]\n(assumed to be in units of cm / 'scaling') by a factor of " +
            Moose::stringify(centroid_mesh(0) / centroid_template(0)) + ".\n\nDid you forget that the 'mesh_template' must be in "
            "units of centimeters, even when using the 'scaling' parameter?");
      }

      // check if centroids are the same
      bool different_centroids = false;
      for (unsigned int j = 0; j < DIMENSION; ++j)
        different_centroids = different_centroids || !MooseUtils::absoluteFuzzyEqual(centroid_mesh(j), centroid_template(j));

      if (different_centroids)
        mooseError("Centroid for element " + Moose::stringify(offset + e) + " in the [Mesh]: (" +
          Moose::stringify(centroid_mesh(0)) + ", " + Moose::stringify(centroid_mesh(1)) + ", " +
          Moose::stringify(centroid_mesh(2)) + ")\ndoes not match centroid for element " + Moose::stringify(e) +
          " in 'mesh_template' " + Moose::stringify(i) + ": (" +
          Moose::stringify(centroid_template(0)) + ", " + Moose::stringify(centroid_template(1)) + ", " +
          Moose::stringify(centroid_template(2)) + ")!\n\nThe copy transfer requires that the [Mesh] and " +
          "'mesh_template' be identical (except for a factor of 'scaling').");
    }

    offset += filter->n_bins();
  }

}

void
OpenMCCellAverageProblem::readMeshTranslations(const std::vector<std::vector<double>> & data)
{
  for (const auto & d : data)
  {
    if (d.size() != DIMENSION)
      paramError("mesh_translations_file", "All entries in 'mesh_translations_file' "
        "must contain exactly ", DIMENSION, " coordinates.");

    // DIMENSION will always be 3
    _mesh_translations.push_back(Point(d[0], d[1], d[2]));
  }
}

void
OpenMCCellAverageProblem::checkBlockOverlap()
{
  std::vector<SubdomainID> intersection;
  std::set_intersection(_fluid_blocks.begin(), _fluid_blocks.end(), _solid_blocks.begin(),
    _solid_blocks.end(), std::back_inserter(intersection));

  if (intersection.size() != 0)
    mooseError("Block " + Moose::stringify(intersection[0]) + " cannot be present in both the 'fluid_blocks' and 'solid_blocks'!");
}

void
OpenMCCellAverageProblem::getCellLevel(const std::string name, int & cell_level)
{
  std::string param_name = name + "_cell_level";

  if (isParamValid(name + "_blocks"))
  {
    if (!isParamValid(param_name))
      paramError(param_name, "When specifying " + name + " blocks for coupling, the "
        "coordinate level must be specified!");

    cell_level = getParam<int>(param_name);

    if (cell_level >= openmc::model::n_coord_levels)
      paramError(param_name, "Coordinate level for finding cells cannot be greater than total number "
        "of coordinate levels: " + Moose::stringify(openmc::model::n_coord_levels) + "!");
  }
  else if (isParamValid(param_name))
    mooseWarning("Without setting any '" + name + "_blocks', the '" + name + "_cell_level' parameter is unused!");
}

void
OpenMCCellAverageProblem::readBlockParameters(const std::string name, std::unordered_set<SubdomainID> & blocks)
{
  std::string param_name = name + "_blocks";

  if (isParamValid(param_name))
  {
    std::vector<SubdomainID> b = getParam<std::vector<SubdomainID>>(param_name);
    checkEmptyVector(b, param_name);

    std::copy(b.begin(), b.end(), std::inserter(blocks, blocks.end()));

    const auto & subdomains = _mesh.meshSubdomains();
    for (const auto & b : blocks)
      if (subdomains.find(b) == subdomains.end())
        mooseError("Block " + Moose::stringify(b) + " specified in '" + name + "_blocks' not found in mesh!");
  }
}

int
OpenMCCellAverageProblem::digits(const int & number) const
{
  return std::to_string(number).length();
}

void
OpenMCCellAverageProblem::storeElementPhase()
{
  for (unsigned int e = 0; e < _mesh.nElem(); ++e)
  {
    const auto * elem = _mesh.elemPtr(e);
    auto subdomain_id = elem->subdomain_id();

    if (_fluid_blocks.count(subdomain_id))
      _elem_phase.push_back(coupling::density_and_temperature);
    else if (_solid_blocks.count(subdomain_id))
      _elem_phase.push_back(coupling::temperature);
    else
      _elem_phase.push_back(coupling::none);
  }

  _n_moose_solid_elems = std::count(_elem_phase.begin(), _elem_phase.end(), coupling::temperature);
  _n_moose_fluid_elems = std::count(_elem_phase.begin(), _elem_phase.end(), coupling::density_and_temperature);
  _n_moose_none_elems = std::count(_elem_phase.begin(), _elem_phase.end(), coupling::none);
}

void
OpenMCCellAverageProblem::computeCellMappedVolumes()
{
  for (const auto & c : _cell_to_elem)
  {
    auto cell_info = c.first;

    _cell_to_elem_volume[cell_info] = 0.0;

    for (const auto & e : c.second)
      _cell_to_elem_volume[cell_info] += _mesh.elemPtr(e)->volume();
  }
}

const coupling::CouplingFields
OpenMCCellAverageProblem::cellCouplingFields(const cellInfo & cell_info)
{
  return _elem_phase[_cell_to_elem[cell_info][0]];
}

void
OpenMCCellAverageProblem::checkCellMappedPhase()
{
  // whether the entire problem has identified any fluid or solid cells
  bool has_fluid_cells = false;
  bool has_solid_cells = false;

  // whether each cell maps to a single phase
  for (const auto & c: _cell_to_elem)
  {
    int n_solid = 0, n_fluid = 0, n_none = 0;
    auto cell_info = c.first;

    for (const auto & e : c.second)
    {
      switch (_elem_phase[e])
      {
        case coupling::temperature:
          has_solid_cells = true;
          n_solid++;
          break;
        case coupling::density_and_temperature:
          has_fluid_cells = true;
          n_fluid++;
          break;
        case coupling::none:
          n_none++;
          break;
        default:
          mooseError("Unhandled CouplingFieldsEnum in OpenMCCellAverageProblem!");
      }
    }

    std::stringstream msg;
      msg << printCell(cell_info) << ": " << std::setw(digits(_n_moose_solid_elems)) << Moose::stringify(n_solid) <<
      " solid elems  " << std::setw(digits(_n_moose_fluid_elems)) << Moose::stringify(n_fluid) <<
      " fluid elems  " << std::setw(digits(_n_moose_none_elems)) << Moose::stringify(n_none) <<
      " uncoupled elems  |  Mapped elems volume (cm3): " << std::setw(8) <<
      Moose::stringify(_cell_to_elem_volume[cell_info] * _scaling * _scaling * _scaling);

    std::vector<bool> conditions = {n_fluid > 0, n_solid > 0, n_none > 0};
    if (std::count(conditions.begin(), conditions.end(), true) > 1)
      mooseError(msg.str() + "\n\n Each OpenMC cell, instance pair must map to elements of the same phase.");

    if (_verbose)
      _console << msg.str() << std::endl;
  }

  // print newline to keep output neat between output sections
  if (_verbose)
    _console << std::endl;

  if (_has_fluid_blocks && !has_fluid_cells)
    mooseError("'fluid_blocks' was specified, but no fluid elements mapped to OpenMC cells!");

  if (_has_solid_blocks && !has_solid_cells)
    mooseError("'solid_blocks' was specified, but no solid elements mapped to OpenMC cells!");
}

void
OpenMCCellAverageProblem::checkCellMappedSubdomains()
{
  for (const auto & c : _cell_to_elem)
  {
    // find the set of subdomains that this cell maps to
    std::set<SubdomainID> cell_to_elem_subdomain;
    for (const auto & e : c.second)
    {
      const auto * elem = _mesh.elemPtr(e);
      cell_to_elem_subdomain.insert(elem->subdomain_id());
    }

    // If the OpenMC cell maps to multiple subdomains that _also_ have different
    // tally settings, we need to error because we are unsure of whether to add tallies or not;
    // both of these need to be true to error
    bool at_least_one_in_tallies = false;
    bool at_least_one_not_in_tallies = false;
    int block_in_tallies, block_not_in_tallies;
    for (const auto & s : cell_to_elem_subdomain)
    {
      if (!at_least_one_in_tallies)
      {
        at_least_one_in_tallies = _tally_blocks.count(s) != 0;
        block_in_tallies = s;
      }

      if (!at_least_one_not_in_tallies)
      {
        at_least_one_not_in_tallies = _tally_blocks.count(s) == 0;
        block_not_in_tallies = s;
      }

      // can cut the search early if we've already hit multiple tally settings
      if (at_least_one_in_tallies && at_least_one_not_in_tallies)
        break;
    }

    const auto cell_info = c.first;

    if (at_least_one_in_tallies && at_least_one_not_in_tallies)
      mooseError(printCell(cell_info) + " maps to blocks with different tally settings!\n"
        "Block " + Moose::stringify(block_in_tallies) + " is in 'tally_blocks', but "
        "block " + Moose::stringify(block_not_in_tallies) + " is not.");

    _cell_has_tally[cell_info] = at_least_one_in_tallies;
  }
}

void
OpenMCCellAverageProblem::getMaterialFills()
{
  std::set<int32_t> materials;

  for (const auto & c : _cell_to_elem)
  {
    auto cell_info = c.first;

    // skip if the cell isn't fluid
    if (cellCouplingFields(cell_info) != coupling::density_and_temperature)
      continue;

    int fill_type;
    std::vector<int32_t> material_indices = cellFill(cell_info, fill_type);

    // OpenMC checks that for distributed cells, the number of materials either equals 1
    // or the number of distributed cells; therefore, we just need to index based on the cell
    // instance (zero for not-distributed cells, otherwise matches the material index)
    int32_t material_index = material_indices[cell_info.second];
    _cell_to_material[cell_info] = material_index;

    if (_verbose)
      _console << printCell(cell_info) << " mapped to " + printMaterial(material_index) << std::endl;

    // check for each material that we haven't already discovered it; if we have, this means we
    // didnt set up the materials correctly
    if (materials.find(material_index) == materials.end())
      materials.insert(material_index);
    else
      mooseError(printMaterial(material_index) + " is present in more than one "
        "fluid cell.\nThis means that your model cannot independently change the density in cells filled "
        "with this material.");
  }
}

int32_t
OpenMCCellAverageProblem::cellID(const int32_t index) const
{
  int32_t id;
  int err = openmc_cell_get_id(index, &id);

  if (err)
    mooseError("In attempting to get ID for cell with index " + Moose::stringify(index) +
      " , OpenMC reported:\n\n" + std::string(openmc_err_msg));

  return id;
}

int32_t
OpenMCCellAverageProblem::materialID(const int32_t index) const
{
  int32_t id;
  int err = openmc_material_get_id(index, &id);

  if (err)
  {
    std::stringstream msg;
    msg << "In attempting to get ID for material with index " + Moose::stringify(index) +
      ", OpenMC reported:\n\n" + std::string(openmc_err_msg);
  }

  return id;
}

std::string
OpenMCCellAverageProblem::printCell(const cellInfo & cell_info) const
{
  int32_t id = cellID(cell_info.first);

  std::stringstream msg;
  msg << "cell " << std::setw(_n_cell_digits) << Moose::stringify(id) <<
   ", instance " << std::setw(_n_cell_digits) << Moose::stringify(cell_info.second) <<
   " (of " << std::setw(_n_cell_digits) << Moose::stringify(openmc::model::cells[cell_info.first]->n_instances_) << ")";

  return msg.str();
}

std::string
OpenMCCellAverageProblem::printMaterial(const int32_t & index) const
{
  int32_t id = materialID(index);
  std::stringstream msg;
  msg << "material " << id;
  return msg.str();
}

void
OpenMCCellAverageProblem::initializeElementToCellMapping()
{
  if (_specified_scaling)
    _console << "Multiplying mesh coordinates by " + Moose::stringify(_scaling) +
      " to convert to OpenMC's length scale of centimeters" << std::endl;

  /* We consider five different cases here based on how the MOOSE and OpenMC
   * domains might overlap in space:
   *
   * 1: Perfect overlap, every MOOSE element maps to an OpenMC cell and every
   *    OpenMC cell maps to MOOSE element(s)
   *
   * 2: MOOSE domain fully encloses the OpenMC domain, so that not every MOOSE
   *    element maps to an OpenMC cell, but every OpenMC cell maps to a MOOSE element
   *
   * 3: OpenMC domain fully encloses the MOOSE domain, so that not every OpenMC
   *    cell maps to MOOSE element(s), but every MOOSE element maps to an OpenMC cell
   *
   * 4: MOOSE and OpenMC domains only partially overlap, so that not every MOOSE
   *    element maps to an OpenMC and not every OpenMC cell maps to MOOSE element(s)
   *
   * 5: The MOOSE and OpenMC domains do not overlap at all, so no MOOSE elements
   *    map to OpenMC cells and no OpenMC cells map to MOOSE elements.
   *
   * We consider situation #5 to be an error, while the others are technically allowed.
   * We need to error here before getting to OpenMC where we don't map to any cells but
   * would still try to set a cell filter based on no cells.
   */

  // Find cell for each element in the mesh based on the element's centroid
  int n_mapped_solid_elems = 0;
  int n_mapped_fluid_elems = 0;
  int n_mapped_none_elems = 0;

  Real uncoupled_volume = 0.0;

  { // scope only exists for the timed print
    CONTROLLED_CONSOLE_TIMED_PRINT(0.0, 1.0, "Initializing mapping between " + Moose::stringify(_mesh.nElem()) +
      " MOOSE elements and " + Moose::stringify(_n_openmc_cells) + " OpenMC cells (on " +
      Moose::stringify(openmc::model::n_coord_levels) + " coordinate levels)");

    // First, figure out the phase of each element according to the blocks defined by the user
    storeElementPhase();
    for (unsigned int e = 0; e < _mesh.nElem(); ++e)
    {
      const auto * elem = _mesh.elemPtr(e);

      const Point & c = elem->centroid();
      Real element_volume = elem->volume();

      bool error = findCell(c);

      // if we didn't find an OpenMC cell here, then we certainly have an uncoupled region
      if (error)
      {
        _elem_to_cell.push_back({UNMAPPED, UNMAPPED});
        uncoupled_volume += element_volume;
        n_mapped_none_elems++;
        continue;
      }

      // otherwise, this region may potentially map to OpenMC if we _also_ turned
      // on coupling for this region; first, determine the phase of this element
      // and store the information
      int level;

      switch (_elem_phase[e])
      {
        case coupling::density_and_temperature:
        {
          level = _fluid_cell_level;
          n_mapped_fluid_elems++;
          break;
        }
        case coupling::temperature:
        {
          level = _solid_cell_level;
          n_mapped_solid_elems++;
          break;
        }
       case coupling::none:
       {
         uncoupled_volume += element_volume;
         n_mapped_none_elems++;

         // we will succeed in finding a valid cell here; for uncoupled regions,
         // cell_index and cell_instance are unused, so this is just to proceed with program logic
         level = 0;
         break;
       }
       default:
         mooseError("Unhandled CouplingFields enum!");
     }

      if (level > _particle.n_coord() - 1)
      {
        std::string phase = _fluid_blocks.count(elem->subdomain_id()) ? "fluid" : "solid";
        mooseError("Requested coordinate level of " + Moose::stringify(level) + " for the " + phase +
          " exceeds number of nested coordinate levels at (" + Moose::stringify(c(0)) + ", " +
          Moose::stringify(c(1)) + ", " + Moose::stringify(c(2)) + "): " +
          Moose::stringify(_particle.n_coord()));
      }

      auto cell_index = _particle.coord(level).cell;

      // TODO: this is the cell instance at the lowest level in the geometry, which does
      // not necessarily match the "level" supplied on the line above
      auto cell_instance = _particle.cell_instance();

      cellInfo cell_info = {cell_index, cell_instance};

      _elem_to_cell.push_back(cell_info);

      // store the map of cells to elements that will be coupled
      if (_elem_phase[e] != coupling::none)
        _cell_to_elem[cell_info].push_back(e);
    }
  }

  if (_cell_to_elem.size() == 0)
    mooseError("Did not find any overlap between MOOSE elements and OpenMC cells for "
      "the specified blocks!");

  int solid_digits = std::max(digits(_n_moose_solid_elems), digits(n_mapped_solid_elems));
  int fluid_digits = std::max(digits(_n_moose_fluid_elems), digits(n_mapped_fluid_elems));
  int none_digits = std::max(digits(_n_moose_none_elems), digits(n_mapped_none_elems));

  std::stringstream msg;
  msg << " MOOSE mesh has:      " <<
    std::setw(solid_digits) << Moose::stringify(_n_moose_solid_elems) << " solid elems  " <<
    std::setw(fluid_digits) << Moose::stringify(_n_moose_fluid_elems) << " fluid elems  " <<
    std::setw(none_digits) << Moose::stringify(_n_moose_none_elems) << " uncoupled elems\n" <<
    " OpenMC cells map to: " <<
    std::setw(solid_digits) << Moose::stringify(n_mapped_solid_elems) << " solid elems  " <<
    std::setw(fluid_digits) << Moose::stringify(n_mapped_fluid_elems) << " fluid elems  " <<
    std::setw(none_digits) << Moose::stringify(n_mapped_none_elems) << " uncoupled elems" << std::endl;

  _console << msg.str() << std::endl;

  if (_n_moose_solid_elems && (n_mapped_solid_elems != _n_moose_solid_elems))
   mooseWarning("The MOOSE mesh has " + Moose::stringify(_n_moose_solid_elems) + " solid elements, "
     "but only " + Moose::stringify(n_mapped_solid_elems) + " got mapped to OpenMC cells.");

  if (_n_moose_fluid_elems && (n_mapped_fluid_elems != _n_moose_fluid_elems))
   mooseWarning("The MOOSE mesh has " + Moose::stringify(_n_moose_fluid_elems) + " fluid elements, "
     "but only " + Moose::stringify(n_mapped_fluid_elems) + " got mapped to OpenMC cells.");

  if (n_mapped_none_elems)
    mooseWarning("Skipping multiphysics feedback for " + Moose::stringify(n_mapped_none_elems) + " MOOSE elements, " +
      "which occupy a volume of (cm3): " + Moose::stringify(uncoupled_volume * _scaling * _scaling * _scaling));

  // If there is a single coordinate level, we can print a helpful message if there are uncoupled
  // cells in the domain
  if (_single_coord_level)
  {
    int n_uncoupled_cells = _n_openmc_cells - _cell_to_elem.size();
    if (n_uncoupled_cells)
      mooseWarning("Skipping multiphysics feedback for " + Moose::stringify(n_uncoupled_cells) + " OpenMC cells");
  }

  // Compute the volume that each OpenMC cell maps to in the MOOSE mesh
  computeCellMappedVolumes();

  // Check that each cell maps to a single phase
  checkCellMappedPhase();

  // Check that each cell maps to subdomain IDs that all have the same tally setting,
  // and then store which cells should have tallies added to them
  if (_tally_type == tally::cell)
  {
    checkCellMappedSubdomains();
    storeTallyCells();
  }
}

void
OpenMCCellAverageProblem::storeTallyCells()
{
  std::stringstream warning;
  bool print_warning = false;

  for (const auto & c : _cell_to_elem)
  {
    auto cell_info = c.first;

    if (_cell_has_tally[cell_info])
    {
      // if the cell doesn't have fissile material, don't add a tally to save some evaluation
      if (!cellHasFissileMaterials(cell_info))
      {
        // for the special case of a single coordinate level, just silently skip adding the tallies
        if (_using_default_tally_blocks)
          continue;

        // otherwise, warn the user that they've specified tallies for some non-fissile cells
        print_warning = true;
        warning << "\n  " << printCell(cell_info);
      }

      _tally_cells.push_back(cell_info);
    }
  }

  if (print_warning)
    mooseWarning("Skipping tallies for: " + warning.str() +
      "\n\nThese cells do not contain fissile material, but tallies are still specified in 'tally_blocks'.");

  // print newline to keep output neat between output sections
  if (_verbose) _console << std::endl;
}

void
OpenMCCellAverageProblem::addLocalTally(std::vector<openmc::Filter *> & filters, const openmc::TallyEstimator estimator)
{
  auto tally = openmc::Tally::create();
  tally->set_scores({"kappa-fission"});
  tally->estimator_ = estimator;
  tally->set_filters(filters);
  _local_tally.push_back(tally);
}

void
OpenMCCellAverageProblem::initializeTallies()
{
  // create the global tally for normalization
  if (_needs_global_tally)
  {
    _global_tally = openmc::Tally::create();
    _global_tally->set_scores({"kappa-fission"});
  }

  // create the local heating tally
  switch (_tally_type)
  {
    case tally::cell:
    {
      CONTROLLED_CONSOLE_TIMED_PRINT(0.0, 1.0, "Adding cell tallies to blocks " + Moose::stringify(_tally_blocks) + " for " +
        Moose::stringify(_tally_cells.size()) + " cells");

      switch (_tally_filter)
      {
        case filter::cell:
        {
          auto cell_filter = dynamic_cast<openmc::CellFilter *>(openmc::Filter::create("cell"));

          std::vector<int32_t> cell_ids;
          for (const auto & c: _tally_cells)
            cell_ids.push_back(c.first);

          cell_filter->set_cells(cell_ids);
          std::vector<openmc::Filter *> tally_filters = {cell_filter};
          addLocalTally(tally_filters, openmc::TallyEstimator::TRACKLENGTH);
          break;
        }
        case filter::cell_instance:
        {
          auto cell_filter = dynamic_cast<openmc::CellInstanceFilter *>(openmc::Filter::create("cellinstance"));

          std::vector<openmc::CellInstance> cells;
          for (const auto & c : _tally_cells)
            cells.push_back({gsl::narrow_cast<gsl::index>(c.first), gsl::narrow_cast<gsl::index>(c.second)});

          cell_filter->set_cell_instances(cells);
          std::vector<openmc::Filter *> tally_filters = {cell_filter};
          addLocalTally(tally_filters, openmc::TallyEstimator::TRACKLENGTH);
          break;
        }
        default:
          mooseError("Unhandled 'CellFilterEnum' in 'OpenMCCellAverageProblem'!");
      }
      break;
    }
    case tally::mesh:
    {
      { // scope only exists for the console timed print
        CONTROLLED_CONSOLE_TIMED_PRINT(0.0, 1.0, "Adding mesh tally based on " + _mesh_template_filename + " at " +
          Moose::stringify(_mesh_translations.size()) + " locations");

        // create a new mesh; by setting the ID to -1, OpenMC will automatically detect the
        // next available ID
        auto mesh = std::make_unique<openmc::LibMesh>(_mesh_template_filename);
        mesh->set_id(-1);
        mesh->output_ = false;

        int32_t mesh_index = openmc::model::meshes.size();

        _mesh_template = mesh.get();
        openmc::model::meshes.push_back(std::move(mesh));

        for (unsigned int i = 0; i < _mesh_translations.size(); ++i)
        {
          const auto & translation = _mesh_translations[i];
          auto meshFilter = dynamic_cast<openmc::MeshFilter*>(openmc::Filter::create("mesh"));
          meshFilter->set_mesh(mesh_index);
          meshFilter->set_translation({translation(0), translation(1), translation(2)});

          _mesh_filters.push_back(meshFilter);
          std::vector<openmc::Filter *> tally_filters = {meshFilter};
          addLocalTally(tally_filters, openmc::TallyEstimator::COLLISION);
        }
      }

      if (_verbose)
      {
        for (unsigned int i = 0; i < _mesh_translations.size(); ++i)
        {
          const auto & translation = _mesh_translations[i];

          Real volume = 0.0;
          for (decltype(_local_tally.at(i)->n_filter_bins()) e = 0; e < _local_tally.at(i)->n_filter_bins(); ++e)
            volume += _mesh_template->volume(e);

          _console << " Mesh translated to (" <<
            std::setprecision(4) << std::setw(8) << translation(0) << ", " <<
            std::setprecision(4) << std::setw(8) << translation(1) << ", " <<
            std::setprecision(4) << std::setw(8) << translation(2) << "): " <<
            std::setw(6) << _local_tally.at(i)->n_filter_bins() << " elements  |  volume (cm3): " << std::setw(6) << volume << std::endl;
        }
      }

      // TODO: can add the assume_separate setting for a bit of additional performance
      // if we find that we need it

      // if using a mesh tally, we are restricted to collision estimators; therefore,
      // because we are going to use this global tally for normalization, we need to make
      // sure it also uses a collision estimator
      if (_global_tally)
        _global_tally->estimator_ = openmc::TallyEstimator::COLLISION;

      break;
    }
    default:
      mooseError("Unhandled TallyTypeEnum in OpenMCCellAverageProblem!");
  }

  // if the tally sum check is turned off, write a message informing the user
  if (!_check_tally_sum)
    _console << " Turned OFF tally sum check against global tally" << std::endl;
}

bool
OpenMCCellAverageProblem::findCell(const Point & point)
{
  _particle.clear();
  _particle.r() = {point(0) * _scaling, point(1) * _scaling, point(2) * _scaling};
  _particle.u() = {0., 0., 1.};

  return !openmc::exhaustive_find_cell(_particle);
}

double
OpenMCCellAverageProblem::tallySum(std::vector<openmc::Tally *> tally) const
{
  double sum = 0.0;

  for (const auto & t : tally)
  {
    auto mean = xt::view(t->results_, xt::all(), 0, static_cast<int>(openmc::TallyResult::SUM));
    sum += xt::sum(mean)();
  }

  return sum;
}

void OpenMCCellAverageProblem::addExternalVariables()
{
  auto var_params = _factory.getValidParams("MooseVariable");
  var_params.set<MooseEnum>("family") = "MONOMIAL";
  var_params.set<MooseEnum>("order") = "CONSTANT";

  addAuxVariable("MooseVariable", "heat_source", var_params);
  _heat_source_var = _aux->getFieldVariable<Real>(0, "heat_source").number();

  addAuxVariable("MooseVariable", "temp", var_params);
  _temp_var = _aux->getFieldVariable<Real>(0, "temp").number();

  // we need a density variable if we are transferring density into OpenMC
  if (_has_fluid_blocks)
  {
    addAuxVariable("MooseVariable", "density", var_params);
    _density_var = _aux->getFieldVariable<Real>(0, "density").number();
  }
}

void OpenMCCellAverageProblem::externalSolve()
{
  int err = openmc_run();
  if (err)
    mooseError(openmc_err_msg);
}

void
OpenMCCellAverageProblem::sendTemperatureToOpenMC()
{
  const auto sys_number = _aux->number();
  const auto & mesh = _mesh.getMesh();

  _console << "Sending temperature to OpenMC cells... " << printNewline();

  double maximum = std::numeric_limits<double>::min();
  double minimum = std::numeric_limits<double>::max();

  for (const auto & c : _cell_to_elem)
  {
    Real average_temp = 0.0;
    auto cell_info = c.first;

    for (const auto & e : c.second)
    {
      auto elem_ptr = mesh.query_elem_ptr(e);

      if (elem_ptr)
      {
        auto dof_idx = elem_ptr->dof_number(sys_number, _temp_var, 0);
        average_temp += (*_serialized_solution)(dof_idx) * elem_ptr->volume();
      }
    }

    average_temp /= _cell_to_elem_volume[cell_info];

    minimum = std::min(minimum, average_temp);
    maximum = std::max(maximum, average_temp);

    if (_verbose)
      _console << "Setting " << printCell(cell_info) << " to temperature (K): " << std::setw(4) << average_temp << std::endl;

    int err = openmc_cell_set_temperature(cell_info.first, average_temp, &cell_info.second, true);

    // TODO: could add the option to truncate temperatures if we exceed bounds?

    if (err)
      mooseError("In attempting to set " + printCell(cell_info) + " to temperature " +
        Moose::stringify(average_temp) + " (K), OpenMC reported:\n\n" + std::string(openmc_err_msg));
  }

  if (!_verbose)
    _console << "done. Sent cell-averaged min/max (K): " << minimum << ", " << maximum;
  _console << std::endl;
}

void
OpenMCCellAverageProblem::sendDensityToOpenMC()
{
  const auto sys_number = _aux->number();
  const auto & mesh = _mesh.getMesh();

  _console << "Sending density to OpenMC cells... " << printNewline();

  double maximum = std::numeric_limits<double>::min();
  double minimum = std::numeric_limits<double>::max();

  for (const auto & c : _cell_to_elem)
  {
    Real average_density = 0.0;
    auto cell_info = c.first;

    // skip if the cell isn't fluid
    if (cellCouplingFields(cell_info) != coupling::density_and_temperature)
      continue;

    for (const auto & e : c.second)
    {
      auto elem_ptr = mesh.query_elem_ptr(e);

      if (elem_ptr)
      {
        auto dof_idx = elem_ptr->dof_number(sys_number, _density_var, 0);
        average_density += (*_serialized_solution)(dof_idx) * elem_ptr->volume();
      }
    }

    average_density /= _cell_to_elem_volume[cell_info];

    minimum = std::min(minimum, average_density);
    maximum = std::max(maximum, average_density);

    // OpenMC technically allows a density of >= 0.0, but we can impose a tighter
    // check here with a better error message than the Excepts() in material->set_density
    // because it could be a very common mistake to forget to set an initial condition
    // for density if OpenMC runs first
    if (average_density <= 0.0)
      mooseError("Densities less than or equal to zero cannot be set in the OpenMC model!\n " + printCell(cell_info) +
        " set to density " + Moose::stringify(average_density) + " (kg/m3)");

    if (_verbose)
      _console << "Setting " << printCell(cell_info) << " to density (kg/m3): " << std::setw(4) << average_density << std::endl;

    int fill_type;
    std::vector<int32_t> material_indices = cellFill(cell_info, fill_type);

    // throw a special error if the cell is void, because the OpenMC error isn't very
    // clear what the mistake is
    if (material_indices[0] == MATERIAL_VOID)
      mooseError("Cannot set density for " + printCell(cell_info) +
        " because this cell is void (vacuum)!");

    if (fill_type != static_cast<int>(openmc::Fill::MATERIAL))
      mooseError("Density transfer does not currently support cells filled with universes or lattices!");

    // Multiply density by 0.001 to convert from kg/m3 (the units assumed in the 'density'
    // auxvariable as well as the MOOSE fluid properties module) to g/cm3
    const char * units = "g/cc";
    int err = openmc_material_set_density(material_indices[cell_info.second], average_density * _density_conversion_factor, units);

    if (err)
      mooseError("In attempting to set material with index " + Moose::stringify(material_indices[cell_info.second]) +
        " to density " + Moose::stringify(average_density) + " (kg/m3), OpenMC reported:\n\n" + std::string(openmc_err_msg));
  }

  if (!_verbose)
    _console << "done. Sent cell-averaged min/max (kg/m3): " << minimum << ", " << maximum;
  _console << std::endl;
}

void
OpenMCCellAverageProblem::checkZeroTally(const Real & power_fraction, const std::string & descriptor) const
{
  if (_verbose)
    _console << " " << descriptor << " power fraction: " << std::setw(3) <<
      Moose::stringify(power_fraction) << std::endl;

  if (_check_zero_tallies && power_fraction < 1e-12)
    mooseError("Heat source computed for " + descriptor + " is zero!\n\n" +
      "This may occur if there is no fissile material in this region, if you have very few particles, "
      "or if you have a geometry "
      "setup error. You can turn off this check by setting 'check_zero_tallies' to false.");
}

Real
OpenMCCellAverageProblem::normalizeLocalTally(const Real & tally_result) const
{
  if (_normalize_by_global)
    return tally_result / _global_kappa_fission;
  else
    return tally_result / _local_kappa_fission;
}

void
OpenMCCellAverageProblem::getHeatSourceFromOpenMC()
{
  auto & solution = _aux->solution();
  auto sys_number = _aux->number();
  const auto & mesh = _mesh.getMesh();

  _console << "Extracting OpenMC fission heat source... " << printNewline();

  // get the total kappa fission sources for normalization
  if (_global_tally)
    _global_kappa_fission = tallySum({_global_tally});

  _local_kappa_fission = tallySum(_local_tally);

  if (_check_tally_sum)
    checkTallySum();

  Real power_fraction_sum = 0.0;

  switch (_tally_type)
  {
    case tally::cell:
    {
      auto mean_tally = xt::view(_local_tally.at(0)->results_, xt::all(), 0, static_cast<int>(openmc::TallyResult::SUM));

      int i = 0;
      for (const auto & c : _cell_to_elem)
      {
        auto cell_info = c.first;

        // if this cell doesn't have any tallies, skip it
        if (!_cell_has_tally[cell_info])
          continue;

        Real power_fraction = normalizeLocalTally(mean_tally(i++));

        // divide each tally value by the volume that it corresponds to in MOOSE
        // because we will apply it as a volumetric heat source (W/volume).
        Real volumetric_power = power_fraction * _power / _cell_to_elem_volume[cell_info];
        power_fraction_sum += power_fraction;

        checkZeroTally(power_fraction, printCell(cell_info));

        // loop over all the elements that belong to this cell, and set the heat
        // source to the computed value
        for (const auto & e : c.second)
        {
          auto elem_ptr = mesh.query_elem_ptr(e);
          if (elem_ptr)
          {
            auto dof_idx = elem_ptr->dof_number(sys_number, _heat_source_var, 0);
            solution.set(dof_idx, volumetric_power);
          }
        }
      }
      break;
    }
  case tally::mesh:
  {
    // TODO: this requires that the mesh exactly correspond to the mesh templates;
    // for cases where they don't match, we'll need to do a nearest-node transfer or something

    unsigned int offset = 0;
    for (unsigned int i = 0; i < _mesh_filters.size(); ++i)
    {
      const auto * filter = _mesh_filters[i];
      auto mean_tally = xt::view(_local_tally.at(i)->results_, xt::all(), 0, static_cast<int>(openmc::TallyResult::SUM));

      for (decltype(filter->n_bins()) e = 0; e < filter->n_bins(); ++e)
      {
        auto elem_ptr = mesh.query_elem_ptr(offset + e);

        if (elem_ptr)
        {
          Real power_fraction = normalizeLocalTally(mean_tally(e));

          // divide each tally by the volume that it corresponds to in MOOSE
          // because we will apply it as a volumetric heat source (W/volume).
          // Because we require that the mesh template has units of cm based on the
          // mesh constructors in OpenMC, we need to adjust the division
          Real volumetric_power = power_fraction * _power / _mesh_template->volume(e) *
            _scaling * _scaling * _scaling;
          power_fraction_sum += power_fraction;

          checkZeroTally(power_fraction, "mesh " + Moose::stringify(i) + ", element " + Moose::stringify(e));

          auto dof_idx = elem_ptr->dof_number(sys_number, _heat_source_var, 0);
          solution.set(dof_idx, volumetric_power);
        }
      }

      offset += filter->n_bins();
    }

    break;
  }
  default:
    mooseError("Unhandled TallyTypeEnum in OpenMCCellAverageProblem!");
  }

  if (_check_tally_sum)
    if (std::abs(power_fraction_sum - 1.0) > 1e-6)
      mooseError("Tally normalization process failed! Total power fraction of " +
        Moose::stringify(power_fraction_sum) + " does not match 1.0!");

  solution.close();
  _console << "done" << std::endl;
}

void OpenMCCellAverageProblem::syncSolutions(ExternalProblem::Direction direction)
{
  auto & solution = _aux->solution();

  if (_first_transfer)
    _serialized_solution->init(_aux->sys().n_dofs(), false, SERIAL);

  solution.localize(*_serialized_solution);

  switch (direction)
  {
    case ExternalProblem::Direction::TO_EXTERNAL_APP:
    {
      if (_first_transfer && _skip_first_incoming_transfer)
      {
        std::string incoming_transfer = _has_fluid_blocks ? "temperature and density" : "temperature";
        _console << "Skipping " << incoming_transfer << " transfer into OpenMC" << std::endl;
        return;
      }

      // Because we require at least one of fluid_blocks and solid_blocks, we are guaranteed
      // to be setting the temperature of all of the cells in cell_to_elem - only for the density
      // transfer do we need to filter for the fluid cells
      sendTemperatureToOpenMC();

      if (_has_fluid_blocks)
        sendDensityToOpenMC();

      break;
    }
    case ExternalProblem::Direction::FROM_EXTERNAL_APP:
    {
      getHeatSourceFromOpenMC();

      break;
    }
    default:
      mooseError("Unhandled Direction enum in OpenMCCellAverageProblem!");
  }

  _first_transfer = false;
}

void
OpenMCCellAverageProblem::checkTallySum() const
{
  if (std::abs(_global_kappa_fission - _local_kappa_fission) / _global_kappa_fission > openmc::FP_REL_PRECISION)
  {
    std::stringstream msg;
    msg << "Heating tallies do not match the global kappa-fission tally:\n" <<
      " Global value: " << Moose::stringify(_global_kappa_fission) <<
      "\n Tally sum: " << Moose::stringify(_local_kappa_fission) <<
      "\n\nYou can turn off this check by setting 'check_tally_sum' to false.";

    // Add on extra helpful messages if the domain has a single coordinate level
    // and cell tallies are used
    if (_tally_type == tally::cell && _single_coord_level)
    {
      int n_uncoupled_cells = _n_openmc_cells - _cell_to_elem.size();
      if (n_uncoupled_cells)
        msg << "\n\nYour problem has " + Moose::stringify(n_uncoupled_cells) +
          " uncoupled OpenMC cells; this warning might be caused by these cells contributing\n" +
          "to the global kappa fission tally, without being part of the multiphysics setup.";

      // If there are cells in OpenMC's problem that we're not coupling with (and therefore not
      // adding tallies for), it's possible that some of cells we're excluding have fissile material.
      // This could also be caused by us not turning tallies on for the solid and/or fluid that have fissile
      // material (note that this only catches cases where we added blocks in 'fluid_blocks' and
      // 'solid_blocks', but forgot to add them to the 'tally_blocks'
      bool missing_tallies_in_fluid = _has_fluid_blocks &&
        std::includes(_tally_blocks.begin(), _tally_blocks.end(), _fluid_blocks.begin(), _fluid_blocks.end());

      bool missing_tallies_in_solid = _has_solid_blocks &&
        std::includes(_tally_blocks.begin(), _tally_blocks.end(), _solid_blocks.begin(), _solid_blocks.end());

      if (missing_tallies_in_fluid || missing_tallies_in_solid)
      {
        std::string missing_tallies;

        if (missing_tallies_in_fluid && missing_tallies_in_solid)
          missing_tallies = "fluid and solid";
        else if (missing_tallies_in_fluid)
          missing_tallies = "fluid";
        else if (missing_tallies_in_solid)
          missing_tallies = "solid";

        msg << "\n\nYour problem didn't add tallies for the " << missing_tallies <<
          "; this warning might be caused by\nfission sources in these regions that "
          "contribute to the global kappa fission tally, without being\npart of the multiphysics setup.";
      }
    }

    mooseError(msg.str());
  }
}

std::vector<int32_t>
OpenMCCellAverageProblem::cellFill(const cellInfo & cell_info, int & fill_type) const
{
  fill_type = static_cast<int>(openmc::Fill::MATERIAL);
  int32_t * materials = nullptr;
  int n_materials = 0;

  int err = openmc_cell_get_fill(cell_info.first, &fill_type, &materials, &n_materials);

  if (err)
    mooseError("In attempting to get fill of " + printCell(cell_info) +
      ", OpenMC reported:\n\n" + std::string(openmc_err_msg));

  std::vector<int32_t> material_indices;
  material_indices.assign(materials, materials + n_materials);
  return material_indices;
}

bool
OpenMCCellAverageProblem::cellHasFissileMaterials(const cellInfo & cell_info) const
{
  int fill_type;
  std::vector<int32_t> material_indices = cellFill(cell_info, fill_type);

  // TODO: for cells with non-material fills, we need to implement something that recurses
  // into the cell/universe fills to see if there's anything fissile; until then, just assume
  // that the cell has something fissile
  if (fill_type != static_cast<int>(openmc::Fill::MATERIAL))
    return true;

  // for each material fill, check whether it is fissionable
  for (const auto & index : material_indices)
  {
    // We know void cells certainly aren't fissionable; if not void, check if fissionable
    if (index != MATERIAL_VOID)
    {
      const auto & material = openmc::model::materials[index];
      if (material->fissionable_)
        return true;
    }
  }

  return false;
}
