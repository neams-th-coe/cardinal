#ifdef ENABLE_DAGMC

#include "MoabSkinner.h"
#include "VariadicTable.h"
#include "AuxiliarySystem.h"
#include "BinUtility.h"
#include "GeometryUtils.h"
#include "UserErrorChecking.h"
#include "DisplacedProblem.h"

#include "libmesh/elem.h"
#include "libmesh/enum_io_package.h"
#include "libmesh/enum_order.h"
#include "libmesh/enum_fe_family.h"
#include "libmesh/equation_systems.h"
#include "libmesh/system.h"
#include "libmesh/mesh_tools.h"

registerMooseObject("CardinalApp", MoabSkinner);

InputParameters
MoabSkinner::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params.addParam<bool>("verbose", false, "Whether to print diagnostic information");

  // temperature binning
  params.addRequiredParam<std::string>("temperature",
                                       "Temperature variable by which to bin elements");
  params.addRangeCheckedParam<Real>(
      "temperature_min", 0.0, "temperature_min >= 0.0", "Lower bound of temperature bins");
  params.addRequiredParam<Real>("temperature_max", "Upper bound of temperature bins");
  params.addRequiredRangeCheckedParam<unsigned int>(
      "n_temperature_bins", "n_temperature_bins > 0", "Number of temperature bins");

  // density binning
  params.addParam<std::string>("density", "Density variable by which to bin elements");
  params.addRangeCheckedParam<Real>(
      "density_min", 0.0, "density_min >= 0.0", "Lower bound of density bins");
  params.addParam<Real>("density_max", "Upper bound of density bins");
  params.addRangeCheckedParam<unsigned int>(
      "n_density_bins", "n_density_bins > 0", "Number of density bins");

  params.addParam<std::vector<std::string>>(
      "material_names",
      "List of names for each subdomain to use for naming the new volumes created in MOAB. "
      "You only need to set this if using this skinner independent of OpenMC; otherwise, "
      "these names are auto-deduced from OpenMC");

  params.addRangeCheckedParam<Real>(
      "faceting_tol", 1e-4, "faceting_tol > 0", "Faceting tolerance for DagMC");
  params.addRangeCheckedParam<Real>(
      "geom_tol", 1e-6, "geom_tol > 0", "Geometry tolerance for DagMC");

  params.addParam<bool>(
      "build_graveyard", false, "Whether to build a graveyard around the geometry");
  params.addRangeCheckedParam<Real>(
      "graveyard_scale_inner",
      1.01,
      "graveyard_scale_inner > 1",
      "Multiplier on mesh bounding box to form inner graveyard surface");
  params.addParam<Real>("graveyard_scale_outer",
                        1.10,
                        "Multiplier on mesh bounding box to form outer graveyard surface");

  // TODO: would be nice to support other file formats as well, like exodus
  params.addParam<bool>(
      "output_skins",
      false,
      "Whether the skinned MOAB mesh (skins generated from the "
      "libMesh [Mesh]) should be written to a file. The files will be named moab_skins_<n>.h5m, "
      "where <n> is the time step index. You can then visualize these files by running "
      "'mbconvert'.");
  params.addParam<bool>("output_full",
                        false,
                        "Whether the MOAB mesh (copied from the libMesh [Mesh]) should "
                        "be written to a file. The files will be named moab_full_<n>.h5m, where "
                        "<n> is the time step index. "
                        "You can then visualize these files by running 'mbconvert'.");
  params.addParam<bool>("use_displaced_mesh",
                        false,
                        "Whether the skinned mesh should be generated from a displaced mesh ");
  params.addClassDescription("Re-generate the OpenMC geometry on-the-fly according to changes in "
                             "the mesh geometry and/or contours in temperature and density");
  return params;
}

MoabSkinner::MoabSkinner(const InputParameters & parameters)
  : GeneralUserObject(parameters),
    _serialized_solution(NumericVector<Number>::build(_communicator).release()),
    _verbose(getParam<bool>("verbose")),
    _temperature_name(getParam<std::string>("temperature")),
    _temperature_min(getParam<Real>("temperature_min")),
    _temperature_max(getParam<Real>("temperature_max")),
    _n_temperature_bins(getParam<unsigned int>("n_temperature_bins")),
    _temperature_bin_width((_temperature_max - _temperature_min) / _n_temperature_bins),
    _bin_by_density(isParamValid("density")),
    _faceting_tol(getParam<Real>("faceting_tol")),
    _geom_tol(getParam<Real>("geom_tol")),
    _graveyard_scale_inner(getParam<double>("graveyard_scale_inner")),
    _graveyard_scale_outer(getParam<double>("graveyard_scale_outer")),
    _output_skins(getParam<bool>("output_skins")),
    _output_full(getParam<bool>("output_full")),
    _use_displaced(getParam<bool>("use_displaced_mesh")),
    _scaling(1.0),
    _n_write(0),
    _standalone(true)
{
  _build_graveyard = getParam<bool>("build_graveyard");

  // we can probably support this in the future, it's just not implemented yet
  if (!getMooseMesh().getMesh().is_serial())
    mooseError("MoabSkinner does not yet support distributed meshes!");

  // Create MOAB interface
  _moab = std::make_shared<moab::Core>();

  // Create a skinner
  skinner = std::make_unique<moab::Skinner>(_moab.get());

  // Create a geom topo tool
  gtt = std::make_unique<moab::GeomTopoTool>(_moab.get());

  if (_bin_by_density)
  {
    checkRequiredParam(parameters, "density_min", "binning by density");
    checkRequiredParam(parameters, "density_max", "binning by density");
    checkRequiredParam(parameters, "n_density_bins", "binning by density");

    _density_min = getParam<Real>("density_min");
    _density_max = getParam<Real>("density_max");
    _n_density_bins = getParam<unsigned int>("n_density_bins");
    _density_name = getParam<std::string>("density");
    _density_bin_width = (_density_max - _density_min) / _n_density_bins;

    if (_density_max < _density_min)
      paramError("density_max", "'density_max' must be greater than 'density_min'");
  }
  else
  {
    checkUnusedParam(parameters, "density_min", "not binning by density");
    checkUnusedParam(parameters, "density_max", "not binning by density");
    checkUnusedParam(parameters, "n_density_bins", "not binning by density");

    _n_density_bins = 1;
  }

  if (_build_graveyard)
  {
    if (_graveyard_scale_outer < _graveyard_scale_inner)
      paramError("graveyard_scale_outer",
                 "'graveyard_scale_outer' must be greater than 'graveyard_scale_inner'!");
  }
  else
  {
    checkUnusedParam(parameters, "graveyard_scale_inner", "'build_graveyard' is false");
    checkUnusedParam(parameters, "graveyard_scale_outer", "'build_graveyard' is false");
  }

  // get variable numbers
  _temperature_var_num = getAuxiliaryVariableNumber(_temperature_name, "temperature");
  if (_bin_by_density)
  {
    if (_temperature_name == _density_name)
      mooseError("The 'temperature' and 'density' variables cannot be the same!");

    _density_var_num = getAuxiliaryVariableNumber(_density_name, "density");
  }

  if (_temperature_max <= _temperature_min)
    paramError("temperature_max", "'temperature_max' must be greater than 'temperature_min'");

  for (unsigned int i = 0; i < _n_temperature_bins + 1; ++i)
    _temperature_bin_bounds.push_back(_temperature_min + i * _temperature_bin_width);

  for (unsigned int i = 0; i < _n_density_bins + 1; ++i)
    _density_bin_bounds.push_back(_density_min + i * _density_bin_width);

  // node numberings for first-order tets
  _tet4_nodes.push_back({0, 1, 2, 3});

  // node numbers for second-order tets
  _tet10_nodes.push_back({0, 4, 6, 7});
  _tet10_nodes.push_back({1, 5, 4, 8});
  _tet10_nodes.push_back({2, 6, 5, 9});
  _tet10_nodes.push_back({7, 8, 9, 3});
  _tet10_nodes.push_back({4, 9, 7, 8});
  _tet10_nodes.push_back({4, 5, 9, 8});
  _tet10_nodes.push_back({4, 7, 9, 6});
  _tet10_nodes.push_back({4, 9, 5, 6});

  moab::MBErrorHandler_Init();
}

void
MoabSkinner::finalize()
{
  moab::MBErrorHandler_Finalize();
}

moab::ErrorCode
MoabSkinner::check(const moab::ErrorCode input) const
{
#ifdef DEBUG
  MB_CHK_ERR(input);
#endif
  return moab::MB_SUCCESS;
}

unsigned int
MoabSkinner::getAuxiliaryVariableNumber(const std::string & name,
                                        const std::string & param_name) const
{
  if (!_fe_problem.getAuxiliarySystem().hasVariable(name))
    paramError(param_name, "Cannot find auxiliary variable '", name, "'!");

  // we require these variables to be constant monomial
  auto type = _fe_problem.getAuxiliarySystem().getFieldVariable<Real>(0, name).feType();
  if (type.family != MONOMIAL || type.order != 0)
    paramError(param_name, "Auxiliary variable '", name, "' must be a CONSTANT MONOMIAL type!");

  return _fe_problem.getAuxiliarySystem().getFieldVariable<Real>(0, name).number();
}

MooseMesh &
MoabSkinner::getMooseMesh()
{
  if (_use_displaced && _fe_problem.getDisplacedProblem() == nullptr)
    mooseError("Displaced mesh was requested but the displaced problem does not exist. "
               "set use_displaced_mesh = False");
  return ((_use_displaced && _fe_problem.getDisplacedProblem())
              ? _fe_problem.getDisplacedProblem()->mesh()
              : _fe_problem.mesh());
}

void
MoabSkinner::initialize()
{
  findBlocks();

  if (_standalone)
  {
    checkRequiredParam(
        parameters(), "material_names", "using skinner independent of an OpenMC [Problem]");
    _material_names = getParam<std::vector<std::string>>("material_names");

    if (_material_names.size() != _n_block_bins)
      paramError("material_names",
                 "This parameter must be the same length as the number of "
                 "subdomains in the mesh (" +
                     Moose::stringify(_n_block_bins) + ")");
  }
  else
  {
    // the external class should have set the value of _material_names
    checkUnusedParam(parameters(),
                     "material_names",
                     "using the skinner in conjunction with an OpenMC [Problem]");
  }

  // Set spatial dimension in MOAB
  check(_moab->set_dimension(getMooseMesh().getMesh().spatial_dimension()));

  // Create a meshset representing all of the MOAB tets
  check(_moab->create_meshset(moab::MESHSET_SET, _all_tets));

  createTags();

  createMOABElems();
}

void
MoabSkinner::execute()
{
  if (_standalone)
    update();
}

void
MoabSkinner::update()
{
  _console << "Skinning geometry into " << _n_temperature_bins << " temperature bins, "
           << _n_density_bins << " density bins, and " << _n_block_bins << " block bins... " << std::endl;

  // Clear MOAB mesh data from last timestep
  reset();

  _serialized_solution->init(_fe_problem.getAuxiliarySystem().sys().n_dofs(), false, SERIAL);
  _fe_problem.getAuxiliarySystem().solution().localize(*_serialized_solution);

  // Re-initialise the mesh data
  initialize();

  // Sort libMesh elements into bins
  sortElemsByResults();

  // Find the surfaces of local temperature regions
  findSurfaces();
}

void
MoabSkinner::findBlocks()
{
  _blocks.clear();

  int i = 0;
  for (const auto & b : getMooseMesh().meshSubdomains())
    _blocks[b] = i++;

  _n_block_bins = _blocks.size();
}

void
MoabSkinner::createMOABElems()
{
  // Clear prior results
  _id_to_elem_handles.clear();

  std::map<dof_id_type, moab::EntityHandle> node_id_to_handle;

  double coords[3];

  // Save all the node information
  for (const auto & node : getMooseMesh().getMesh().node_ptr_range())
  {
    // Fetch coords (and scale to correct units)
    coords[0] = _scaling * (*node)(0);
    coords[1] = _scaling * (*node)(1);
    coords[2] = _scaling * (*node)(2);

    // Add node to MOAB database and get handle
    moab::EntityHandle ent(0);
    check(_moab->create_vertex(coords, ent));

    // Save mapping of libMesh IDs to MOAB vertex handles
    node_id_to_handle[node->id()] = ent;
  }

  moab::Range all_elems;

  // Iterate over elements in the mesh
  for (const auto & elem : getMooseMesh().getMesh().active_element_ptr_range())
  {
    auto nodeSets = getTetSets(elem->type());

    // Get the connectivity
    std::vector<dof_id_type> conn_libmesh;
    elem->connectivity(0, libMesh::IOPackage::VTK, conn_libmesh);

    // Loop over sub tets
    for (const auto & nodeSet : nodeSets)
    {
      // Set MOAB connectivity
      std::vector<moab::EntityHandle> conn(NODES_PER_MOAB_TET);
      for (unsigned int i = 0; i < NODES_PER_MOAB_TET; ++i)
      {
        // Get the elem node index of the ith node of the sub-tet
        unsigned int nodeIndex = nodeSet.at(i);
        conn[i] = node_id_to_handle[conn_libmesh.at(nodeIndex)];
      }

      // Create an element in MOAB database
      moab::EntityHandle ent(0);
      check(_moab->create_element(moab::MBTET, conn.data(), NODES_PER_MOAB_TET, ent));

      // Save mapping between libMesh ids and moab handles
      auto id = elem->id();
      if (_id_to_elem_handles.find(id) == _id_to_elem_handles.end())
        _id_to_elem_handles[id] = std::vector<moab::EntityHandle>();

      _id_to_elem_handles[id].push_back(ent);

      // Save the handle for adding to entity sets
      all_elems.insert(ent);
    }
  }

  // Add the elems to the full meshset
  check(_moab->add_entities(_all_tets, all_elems));

  // Save the first elem
  offset = all_elems.front();
}

const std::vector<std::vector<unsigned int>> &
MoabSkinner::getTetSets(ElemType type) const
{
  if (type == TET4)
    return _tet4_nodes;
  else if (type == TET10)
    return _tet10_nodes;
  else
    mooseError("The MoabSkinner can only be used with a tetrahedral [Mesh]!");
}

void
MoabSkinner::createTags()
{
  // First some built-in MOAB tag types
  check(_moab->tag_get_handle(GEOM_DIMENSION_TAG_NAME,
                              1,
                              moab::MB_TYPE_INTEGER,
                              geometry_dimension_tag,
                              moab::MB_TAG_DENSE | moab::MB_TAG_CREAT));

  check(_moab->tag_get_handle(GLOBAL_ID_TAG_NAME,
                              1,
                              moab::MB_TYPE_INTEGER,
                              id_tag,
                              moab::MB_TAG_DENSE | moab::MB_TAG_CREAT));

  check(_moab->tag_get_handle(CATEGORY_TAG_NAME,
                              CATEGORY_TAG_SIZE,
                              moab::MB_TYPE_OPAQUE,
                              category_tag,
                              moab::MB_TAG_SPARSE | moab::MB_TAG_CREAT));

  check(_moab->tag_get_handle(NAME_TAG_NAME,
                              NAME_TAG_SIZE,
                              moab::MB_TYPE_OPAQUE,
                              name_tag,
                              moab::MB_TAG_SPARSE | moab::MB_TAG_CREAT));

  // Some tags needed for DagMC
  check(_moab->tag_get_handle("FACETING_TOL",
                              1,
                              moab::MB_TYPE_DOUBLE,
                              faceting_tol_tag,
                              moab::MB_TAG_SPARSE | moab::MB_TAG_CREAT));

  check(_moab->tag_get_handle("GEOMETRY_RESABS",
                              1,
                              moab::MB_TYPE_DOUBLE,
                              geometry_resabs_tag,
                              moab::MB_TAG_SPARSE | moab::MB_TAG_CREAT));

  // Set the values for DagMC faceting / geometry tolerance tags on the mesh entity set
  check(_moab->tag_set_data(faceting_tol_tag, &_all_tets, 1, &_faceting_tol));
  check(_moab->tag_set_data(geometry_resabs_tag, &_all_tets, 1, &_geom_tol));
}

void
MoabSkinner::createGroup(const unsigned int & id,
                         const std::string & name,
                         moab::EntityHandle & group_set)
{
  check(_moab->create_meshset(moab::MESHSET_SET, group_set));
  setTags(group_set, name, "Group", id, 4);
}

void
MoabSkinner::createVol(const unsigned int & id,
                       moab::EntityHandle & volume_set,
                       moab::EntityHandle group_set)
{
  check(_moab->create_meshset(moab::MESHSET_SET, volume_set));

  setTags(volume_set, "", "Volume", id, 3);

  // Add the volume to group
  check(_moab->add_entities(group_set, &volume_set, 1));
}

void
MoabSkinner::createSurf(const unsigned int & id,
                        moab::EntityHandle & surface_set,
                        moab::Range & faces,
                        const std::vector<VolData> & voldata)
{
  // Create meshset
  check(_moab->create_meshset(moab::MESHSET_SET, surface_set));

  // Set tags
  setTags(surface_set, "", "Surface", id, 2);

  // Add tris to the surface
  check(_moab->add_entities(surface_set, faces));

  // Create entry in map
  surfsToVols[surface_set] = std::vector<VolData>();

  // Add volume to list associated with this surface
  for (const auto & data : voldata)
    updateSurfData(surface_set, data);
}

void
MoabSkinner::updateSurfData(moab::EntityHandle surface_set, const VolData & data)
{
  // Add the surface to the volume set
  check(_moab->add_parent_child(data.vol, surface_set));

  // Set the surfaces sense
  gtt->set_sense(surface_set, data.vol, int(data.sense));

  surfsToVols[surface_set].push_back(data);
}

void
MoabSkinner::setTags(
    moab::EntityHandle ent, std::string name, std::string category, unsigned int id, int dim)
{
  // Set the name tag
  if (name != "")
    setTagData(name_tag, ent, name, NAME_TAG_SIZE);

  // Set the category tag
  if (category != "")
    setTagData(category_tag, ent, category, CATEGORY_TAG_SIZE);

  // Set the dimension tag
  setTagData(geometry_dimension_tag, ent, &dim);

  // Set the id tag
  setTagData(id_tag, ent, &id);
}

void
MoabSkinner::setTagData(moab::Tag tag, moab::EntityHandle ent, std::string data, unsigned int SIZE)
{
  auto namebuf = new char[SIZE];
  memset(namebuf, '\0', SIZE); // fill C char array with null
  strncpy(namebuf, data.c_str(), SIZE - 1);
  check(_moab->tag_set_data(tag, &ent, 1, namebuf));
  delete[] namebuf;
}

void
MoabSkinner::setTagData(moab::Tag tag, moab::EntityHandle ent, void * data)
{
  check(_moab->tag_set_data(tag, &ent, 1, data));
}

unsigned int
MoabSkinner::nBins() const
{
  return _n_block_bins * _n_density_bins * _n_temperature_bins;
}

void
MoabSkinner::sortElemsByResults()
{
  _elem_bins.clear();
  _elem_bins.resize(nBins());

  // accumulate information for printing diagnostics
  std::vector<unsigned int> n_block_hits(_n_block_bins, 0);
  std::vector<unsigned int> n_temp_hits(_n_temperature_bins, 0);
  std::vector<unsigned int> n_density_hits(_n_density_bins, 0);

  for (unsigned int e = 0; e < getMooseMesh().nElem(); ++e)
  {
    const Elem * const elem = getMooseMesh().queryElemPtr(e);
    if (!elem)
      continue;

    Point p = elem->vertex_average();

    // bin by subdomain ID
    auto iMat = getSubdomainBin(elem);
    n_block_hits[iMat] += 1;

    // bin by density
    auto iDenBin = getDensityBin(elem);
    n_density_hits[iDenBin] += 1;

    // bin by temperature
    auto iBin = getTemperatureBin(elem);
    n_temp_hits[iBin] += 1;

    // Sort elem into a bin
    auto iSortBin = getBin(iBin, iDenBin, iMat);
    _elem_bins.at(iSortBin).insert(elem->id());
  }

  if (_verbose)
  {
    VariadicTable<unsigned int, std::string, unsigned int> vtt({"Bin", "Range (K)", "# Elems"});
    VariadicTable<unsigned int, std::string, unsigned int> vtd({"Bin", "Range (kg/m3)", "# Elems"});

    for (unsigned int i = 0; i < _n_temperature_bins; ++i)
      vtt.addRow(i,
                 std::to_string(_temperature_bin_bounds[i]) + " to " +
                     std::to_string(_temperature_bin_bounds[i + 1]),
                 n_temp_hits[i]);

    for (unsigned int i = 0; i < _n_density_bins; ++i)
      vtd.addRow(i,
                 std::to_string(_density_bin_bounds[i]) + " to " +
                     std::to_string(_density_bin_bounds[i + 1]),
                 n_density_hits[i]);

    _console << "\nMapping of Elements to Temperature Bins:" << std::endl;
    vtt.print(_console);
    _console << std::endl;

    if (_bin_by_density)
    {
      _console << "\n\nMapping of Elements to Density Bins:" << std::endl;
      vtd.print(_console);
      _console << std::endl;
    }
  }
}

unsigned int
MoabSkinner::getTemperatureBin(const Elem * const elem) const
{
  auto dof = elem->dof_number(_fe_problem.getAuxiliarySystem().number(), _temperature_var_num, 0);
  auto value = (*_serialized_solution)(dof);

  // TODO: add option to truncate instead
  if ((_temperature_min - value) > BIN_TOLERANCE)
    mooseError("Variable '",
               _temperature_name,
               "' has value below minimum range of bins. "
               "Please decrease 'temperature_min'.\n\n"
               "  value: ",
               value,
               "\n  temperature_min: ",
               _temperature_min);

  if ((value - _temperature_max) > BIN_TOLERANCE)
    mooseError("Variable '",
               _temperature_name,
               "' has value above maximum range of bins. "
               "Please increase 'temperature_max'.\n\n"
               "  value: ",
               value,
               "\n  temperature_max: ",
               _temperature_max);

  return bin_utility::linearBin(value, _temperature_bin_bounds);
}

unsigned int
MoabSkinner::getDensityBin(const Elem * const elem) const
{
  if (!_bin_by_density)
    return 0;

  auto dof = elem->dof_number(_fe_problem.getAuxiliarySystem().number(), _density_var_num, 0);
  auto value = (*_serialized_solution)(dof);

  // TODO: add option to truncate instead
  if ((_density_min - value) > BIN_TOLERANCE)
    mooseError("Variable '",
               _density_name,
               "' has value below minimum range of bins. "
               "Please decrease 'density_min'.\n\n"
               "  value: ",
               value,
               "\n  density_min: ",
               _density_min);

  if ((value - _density_max) > BIN_TOLERANCE)
    mooseError("Variable '",
               _density_name,
               "' has value above maximum range of bins. "
               "Please increase 'density_max'.\n\n"
               "  value: ",
               value,
               "\n  density_max: ",
               _density_max);

  return bin_utility::linearBin(value, _density_bin_bounds);
}

std::string
MoabSkinner::materialName(const unsigned int & block,
                          const unsigned int & density,
                          const unsigned int & temp) const
{
  if (_n_density_bins > 1)
    return "mat:" + _material_names.at(block) + "_" + std::to_string(density);
  else
    return "mat:" + _material_names.at(block);
}

void
MoabSkinner::findSurfaces()
{
  // Find all neighbours in mesh
  getMooseMesh().getMesh().find_neighbors();

  // Counter for volumes
  unsigned int vol_id = 0;

  // Counter for surfaces
  unsigned int surf_id = 0;

  // Loop over material bins
  for (unsigned int iMat = 0; iMat < _n_block_bins; iMat++)
  {
    // Loop over density bins
    for (unsigned int iDen = 0; iDen < _n_density_bins; iDen++)
    {
      // Loop over temperature bins
      for (unsigned int iVar = 0; iVar < _n_temperature_bins; iVar++)
      {
        // Update material name
        auto updated_mat_name = materialName(iMat, iDen, iVar);

        // Create a material group
        int iSortBin = getBin(iVar, iDen, iMat);

        // For DagMC to fill a cell with a material, we first create a group
        // with that name, and then assign it with createVol (called inside findSurface)
        moab::EntityHandle group_set;
        unsigned int group_id = iSortBin + 1;
        createGroup(group_id, updated_mat_name, group_set);

        // Sort elems in this mat-density-temp bin into local regions
        std::vector<moab::Range> regions;
        groupLocalElems(_elem_bins.at(iSortBin), regions);

        // Loop over all regions and find surfaces
        for (const auto & region : regions)
        {
          moab::EntityHandle volume_set;
          findSurface(region, group_set, vol_id, surf_id, volume_set);
        }
      }
    }
  }

  if (_build_graveyard)
    buildGraveyard(vol_id, surf_id);

  // Write MOAB volume and/or skin meshes to file
  write();
}

void
MoabSkinner::write()
{
  // Only write to file on root process
  if (processor_id() != 0)
    return;

  std::string extension = std::to_string(_n_write) + ".h5m";

  if (_output_skins)
  {
    // Generate list of surfaces to write
    std::vector<moab::EntityHandle> surfs;
    for (const auto & itsurf : surfsToVols)
      surfs.push_back(itsurf.first);

    std::string filename = "moab_skins_" + extension;

    if (_verbose)
      _console << "Writing MOAB skins to " << filename << "...";

    check(_moab->write_mesh(filename.c_str(), surfs.data(), surfs.size()));
  }

  if (_output_full)
  {
    std::string filename = "moab_mesh_" + extension;

    if (_verbose)
      _console << "Writing MOAB mesh to " << filename << std::endl;

    check(_moab->write_mesh(filename.c_str()));
  }

  _n_write++;
}

void
MoabSkinner::groupLocalElems(std::set<dof_id_type> elems, std::vector<moab::Range> & localElems)
{
  while (!elems.empty())
  {

    // Create a new local range of moab handles
    moab::Range local;

    // Retrieve and remove the fisrt elem
    auto it = elems.begin();
    dof_id_type next = *it;
    elems.erase(it);

    std::set<dof_id_type> neighbors;
    neighbors.insert(next);

    while (!neighbors.empty())
    {

      std::set<dof_id_type> new_neighbors;

      // Loop over all the new neighbors
      for (auto & next : neighbors)
      {

        // Get the MOAB handles, and add to local set
        // (May be more than one if this libMesh elem has sub-tetrahedra)
        if (_id_to_elem_handles.find(next) == _id_to_elem_handles.end())
          mooseError("No entity handles found for libmesh id.");

        std::vector<moab::EntityHandle> ents = _id_to_elem_handles[next];
        for (const auto ent : ents)
          local.insert(ent);

        // Get the libMesh element
        Elem & elem = getMooseMesh().getMesh().elem_ref(next);

        // How many nearest neighbors (general element)?
        unsigned int NN = elem.n_neighbors();

        // Loop over neighbors
        for (unsigned int i = 0; i < NN; i++)
        {
          const Elem * nnptr = elem.neighbor_ptr(i);
          // If on boundary, some may be null ptrs
          if (nnptr == nullptr)
            continue;

          dof_id_type idnn = nnptr->id();

          // Select only those that are in the current bin
          if (elems.find(idnn) != elems.end())
          {
            new_neighbors.insert(idnn);
            // Remove from those still available
            elems.erase(idnn);
          }
        }
      }

      // Found all the new neighbors, done with current set.
      neighbors = new_neighbors;
    }

    // Save this moab range of local neighbors
    localElems.push_back(local);
  }
}

void
MoabSkinner::reset()
{
  _moab.reset(new moab::Core());
  skinner.reset(new moab::Skinner(_moab.get()));
  gtt.reset(new moab::GeomTopoTool(_moab.get()));

  // Clear entity set maps
  surfsToVols.clear();
}

unsigned int
MoabSkinner::getBin(const unsigned int & i_temp,
                    const unsigned int & i_density,
                    const unsigned int & i_block) const
{
  return _n_temperature_bins * (_n_density_bins * i_block + i_density) + i_temp;
}

void
MoabSkinner::findSurface(const moab::Range & region,
                         moab::EntityHandle group,
                         unsigned int & vol_id,
                         unsigned int & surf_id,
                         moab::EntityHandle & volume_set)
{
  // Create a volume set
  vol_id++;
  createVol(vol_id, volume_set, group);

  // Find surfaces from these regions
  moab::Range tris;  // The tris of the surfaces
  moab::Range rtris; // The tris which are reversed with respect to their surfaces
  skinner->find_skin(0, region, false, tris, &rtris);

  // Create surface sets for the forwards tris
  VolData vdata = {volume_set, Sense::FORWARDS};
  createSurfaces(tris, vdata, surf_id);

  // Create surface sets for the reversed tris
  vdata.sense = Sense::BACKWARDS;
  createSurfaces(rtris, vdata, surf_id);
}

void
MoabSkinner::createSurfaces(moab::Range & faces, VolData & voldata, unsigned int & surf_id)
{
  if (faces.empty())
    return;

  // Loop over the surfaces we have already created
  for (const auto & surfpair : surfsToVols)
  {
    // Local copies of surf/vols
    moab::EntityHandle surf = surfpair.first;
    std::vector<VolData> vols = surfpair.second;

    // First get the entities in this surface
    moab::Range tris;
    check(_moab->get_entities_by_handle(surf, tris));

    // Find any tris that live in both surfs
    moab::Range overlap = moab::intersect(tris, faces);
    if (!overlap.empty())
    {
      // Check if the tris are a subset or the entire surf
      if (tris.size() == overlap.size())
      {
        // Whole surface -> Just need to update the volume relationships
        updateSurfData(surf, voldata);
      }
      else
      {
        // If overlap is subset, subtract shared tris from this surface and create a new shared
        // surface
        check(_moab->remove_entities(surf, overlap));

        // Append our new volume to the list that share this surf
        vols.push_back(voldata);

        // Create a new shared surface
        moab::EntityHandle shared_surf;
        surf_id++;
        createSurf(surf_id, shared_surf, overlap, vols);
      }

      // Subtract from the input list
      for (auto & shared : overlap)
        faces.erase(shared);

      if (faces.empty())
        break;
    }
  }

  if (!faces.empty())
  {
    moab::EntityHandle surface_set;
    std::vector<VolData> voldatavec(1, voldata);
    surf_id++;
    createSurf(surf_id, surface_set, faces, voldatavec);
  }
}

void
MoabSkinner::buildGraveyard(unsigned int & vol_id, unsigned int & surf_id)
{
  // Create the graveyard group
  moab::EntityHandle graveyard;
  unsigned int id = nBins() + 1;
  createGroup(id, "mat:Graveyard", graveyard);

  // Create a volume set
  moab::EntityHandle volume_set;
  createVol(++vol_id, volume_set, graveyard);

  // Set up for the volume data to pass to surfs
  VolData vdata = {volume_set, Sense::FORWARDS};

  // Find a bounding box
  BoundingBox bbox = MeshTools::create_bounding_box(getMooseMesh().getMesh());

  // Build the two cubic surfaces defining the graveyard
  createSurfaceFromBox(
      bbox, vdata, surf_id, false /* normals point into box */, _graveyard_scale_inner);
  createSurfaceFromBox(
      bbox, vdata, surf_id, true /* normals point out of box */, _graveyard_scale_outer);
}

void
MoabSkinner::createSurfaceFromBox(const BoundingBox & box,
                                  const VolData & voldata,
                                  unsigned int & surf_id,
                                  bool normalout,
                                  const Real & factor)
{
  std::vector<moab::EntityHandle> vert_handles = createNodesFromBox(box, factor);

  // Create the tris in 4 groups of 3 (4 open tetrahedra)
  moab::Range tris;
  createCornerTris(vert_handles, 0, 1, 2, 4, normalout, tris);
  createCornerTris(vert_handles, 3, 2, 1, 7, normalout, tris);
  createCornerTris(vert_handles, 6, 4, 2, 7, normalout, tris);
  createCornerTris(vert_handles, 5, 1, 4, 7, normalout, tris);

  moab::EntityHandle surface_set;
  std::vector<VolData> voldatavec(1, voldata);
  surf_id++;
  createSurf(surf_id, surface_set, tris, voldatavec);
}

std::vector<moab::EntityHandle>
MoabSkinner::createNodesFromBox(const BoundingBox & box, const Real & factor) const
{
  std::vector<moab::EntityHandle> vert_handles;

  // Fetch the vertices of the box
  auto verts = geom_utils::boxCorners(box, factor);

  // Array to represent a coord in MOAB
  double coord[3];

  // Create the vertices in MOAB and get the handles
  for (const auto & vert : verts)
  {
    coord[0] = vert(0) * _scaling;
    coord[1] = vert(1) * _scaling;
    coord[2] = vert(2) * _scaling;

    moab::EntityHandle ent;
    check(_moab->create_vertex(coord, ent));
    vert_handles.push_back(ent);
  }

  return vert_handles;
}

void
MoabSkinner::createCornerTris(const std::vector<moab::EntityHandle> & verts,
                              unsigned int corner,
                              unsigned int v1,
                              unsigned int v2,
                              unsigned int v3,
                              bool normalout,
                              moab::Range & surface_tris)
{
  // Create 3 tris stemming from one corner (i.e. an open tetrahedron)
  // Assume first is the central corner, and the others are labelled clockwise looking down on the
  // corner
  unsigned int indices[3] = {v1, v2, v3};

  // Create each tri by a cyclic permutation of indices
  // Values of i1, i2 in the loop: 0,1; 1,2; 2;0
  for (unsigned int i = 0; i < 3; i++)
  {
    int i1 = indices[i % 3];
    int i2 = indices[(i + 1) % 3];
    if (normalout) // anti-clockwise: normal points outwards
      surface_tris.insert(createTri(verts, corner, i2, i1));
    else // clockwise: normal points inwards
      surface_tris.insert(createTri(verts, corner, i1, i2));
  }
}

moab::EntityHandle
MoabSkinner::createTri(const std::vector<moab::EntityHandle> & vertices,
                       unsigned int v1,
                       unsigned int v2,
                       unsigned int v3)
{
  moab::EntityHandle triangle;
  moab::EntityHandle connectivity[3] = {vertices[v1], vertices[v2], vertices[v3]};
  check(_moab->create_element(moab::MBTRI, connectivity, 3, triangle));
  return triangle;
}

void
MoabSkinner::setGraveyard(bool build)
{
  if (build != _build_graveyard)
  {
    std::string original = _build_graveyard ? "true" : "false";
    std::string change = _build_graveyard ? "false" : "true";
    mooseWarning("Overriding graveyard setting from ",
                 original,
                 " to ",
                 change,
                 ".\n"
                 "To hide this warning, set 'build_graveyard = ",
                 change,
                 "'");
  }

  _build_graveyard = build;
}
#endif
