#ifdef ENABLE_DAGMC

// Moose includes
#include "MoabUserObject.h"
#include "OpenMCDensity.h"
#include "ADOpenMCDensity.h"
#include "DisplacedProblem.h"

registerMooseObject("CardinalApp", MoabUserObject);

InputParameters
MoabUserObject::validParams()
{
  InputParameters params = UserObject::validParams();

  // MOAB mesh params
  params.addParam<double>("length_scale", 100.,"Scale factor to convert lengths from MOOSE to MOAB. Default is from metres->centimetres.");

  // Params relating to binning
  // Temperature binning
  params.addParam<std::string>("bin_varname", "", "Variable name by whose results elements should be binned.");
  params.addParam<double>("var_min", 297.5,"Minimum value to define range of bins.");
  params.addParam<double>("var_max", 597.5,"Max value to define range of bins.");
  params.addParam<bool>("logscale", false, "Switch to determine if logarithmic binning should be used.");
  params.addParam<unsigned int>("n_bins", 60, "Number of bins");
  // Density binning
  params.addParam<std::string>("density_name", "", "Variable name for density by whose results elements should be binned.");
  params.addParam<bool>("bin_density", false, "Determine if elements should be additionally binned by material density");
  params.addParam<double>("rel_den_min", -0.1,"Minimum difference in density relative to original material density");
  params.addParam<double>("rel_den_max",  0.1,"Maximum difference in density relative to original material density");
  params.addParam<unsigned int>("n_density_bins", 5, "Number of relative density bins");
   params.addParam<double>("density_scale", 1.,"Scale factor to convert densities from from MOOSE to OpenMC (latter is g/cc).");

  // Mesh metadata
  params.addParam<std::vector<std::string> >("material_names", std::vector<std::string>(), "List of MOOSE material names");
  params.addParam<std::vector<std::string> >("material_openmc_names", std::vector<std::string>(), "List of OpenMC material names");

  // Dagmc params
  params.addParam<double>("faceting_tol",1.e-4,"Faceting tolerance for DagMC");
  params.addParam<double>("geom_tol",1.e-6,"Geometry tolerance for DagMC");
  params.addParam<double>("graveyard_scale_inner",1.01,"Graveyard inner surface scalefactor relative to aligned bounding box.");
  params.addParam<double>("graveyard_scale_outer",1.10,"Graveyard outer surface scalefactor relative to aligned bounding box.");

  // Output params
  params.addParam<bool>("output_skins", false, "Switch to control whether MOAB should write skins to file.");
  params.addParam<bool>("output_full",  false, "Switch to control whether MOAB should write full mesh data to file.");
  params.addParam<std::string>("output_base", "moab_surfs", "Base filename for surface file writes (will be appended by an integer");
  params.addParam<std::string>("output_base_full", "moab_full", "Base filename for full mesh file writes (will be appended by an integer");
  params.addParam<unsigned int>("n_output", 10, "Number of times to write to file if output_skins and/or output_full is true");
  params.addParam<unsigned int>("n_skip", 0, "Number of iterations to skip between writes if output_skins and/or output_full is true and n_output>1");

  return params;
}

// TO-DO automate the supplying of materials
MoabUserObject::MoabUserObject(const InputParameters & parameters) :
  UserObject(parameters),
  _problem_ptr(nullptr),
  lengthscale(getParam<double>("length_scale")),
  densityscale(getParam<double>("density_scale")),
  var_name(getParam<std::string>("bin_varname")),
  logscale(getParam<bool>("logscale")),
  var_min(getParam<double>("var_min")),
  var_max(getParam<double>("var_max")),
  nVarBins(getParam<unsigned int>("n_bins")),
  den_var_name(getParam<std::string>("density_name")),
  rel_den_min(getParam<double>("rel_den_min")),
  rel_den_max(getParam<double>("rel_den_max")),
  nDenBins(getParam<unsigned int>("n_density_bins")),
  mat_names(getParam<std::vector<std::string> >("material_names")),
  openmc_mat_names(getParam<std::vector<std::string> >("material_openmc_names")),
  faceting_tol(getParam<double>("faceting_tol")),
  geom_tol(getParam<double>("geom_tol")),
  scalefactor_inner(getParam<double>("graveyard_scale_inner")),
  scalefactor_outer(getParam<double>("graveyard_scale_outer")),
  output_skins(getParam<bool>("output_skins")),
  output_full(getParam<bool>("output_full")),
  output_base(getParam<std::string>("output_base")),
  output_base_full(getParam<std::string>("output_base_full")),
  n_output(getParam<unsigned int>("n_output")),
  n_period(getParam<unsigned int>("n_skip")+1),
  _init_timer(registerTimedSection("init", 2)),
  _update_timer(registerTimedSection("update", 2)),
  _setsolution_timer(registerTimedSection("setsolution", 2))
{
  // Create MOAB interface
  moabPtr =  std::make_shared<moab::Core>();

  // Create a skinner
  skinner = std::make_unique<moab::Skinner>(moabPtr.get());

  // Create a geom topo tool
  gtt = std::make_unique<moab::GeomTopoTool>(moabPtr.get());

  // Set variables relating to binning
  binElems = !( var_name == "" || mat_names.empty());
  binByDensity = !( den_var_name == "" || !binElems );

  if(binElems){
    // If no alternative names were provided for openmc materials
    // assume they are the same as in MOOSE
    if(openmc_mat_names.empty()){
      openmc_mat_names = mat_names;
    }
    if(openmc_mat_names.size() != mat_names.size() ){
      mooseError("If both are provided, the vectors material_names and material_openmc_names should have identical lengths.");
    }

    if(var_min <= 0.){
      mooseError("var_min out of range! Please pick a value > 0");
    }
    if(var_max <= var_min){
      mooseError("Please pick a value for var_max > var_min");
    }
    bin_width = (var_max-var_min)/double(nVarBins);
    powMin = int(floor(log10(var_min)));
    powMax = int(ceil(log10(var_max)));
    nPow = std::max(powMax-powMin, 1);
    if(nPow > nVarBins && logscale){
      mooseError("Please ensure number of powers for variable is less than the number of bins");
    }
    nMinor = nVarBins/nPow;
    calcMidpoints();

    if(binByDensity){
      if(rel_den_max < rel_den_min){
        mooseError("Relative density bin range is ill-defined");
      }
      if(nDenBins < 1){
        mooseError("Number of density bins must exceed 0.");
      }
    }
    else{
      // Ignore density settings for whatever user set
      rel_den_min=-0.1;
      rel_den_max=0.1;
      nDenBins=1;
    }
    calcDenMidpoints();
  }

  if(scalefactor_inner < 1.0){
    mooseError("Please set graveyard_scale_inner to a value greater than 1");
  }
  if(scalefactor_outer < scalefactor_inner){
    mooseError("Please ensure graveyard_scale_outer exceeds graveyard_scale_inner");
  }

  // Check output file names are sane
  if(output_skins && output_base==""){
    mooseError("Please provide a non-empty string for output_base when output_skins=true.");
  }
  if(output_full && output_base_full==""){
    mooseError("Please provide a non-empty string for output_base_full when output_full=true.");
  }
  if(output_full && output_skins && output_base_full==output_base){
    mooseError("Please provide unique non-empty strings for output_base_full and output_base.");
  }

  // Set variables relating to writing to file
  n_write=0;
  n_its=0;
}

FEProblemBase&
MoabUserObject::problem()
{
  if(_problem_ptr == nullptr)
    mooseError("No problem was set");

  return *_problem_ptr;
}

MeshBase&
MoabUserObject::mesh()
{
  if(problem().haveDisplaced()){
    return problem().getDisplacedProblem()->mesh().getMesh();
  }
  return problem().mesh().getMesh();
}

EquationSystems&
MoabUserObject::systems()
{
  return problem().es();
}

System&
MoabUserObject::system(std::string var_now)
{
  return problem().getSystem(var_now);
}

void
MoabUserObject::initMOAB()
{

  TIME_SECTION(_init_timer);

  // Fetch spatial dimension from libMesh
  int dim = mesh().spatial_dimension() ;

  // Set spatial dimension in MOAB
  moab::ErrorCode  rval = moabPtr->set_dimension(dim);
  if(rval!=moab::MB_SUCCESS)
    mooseError("Failed to set MOAB dimension");

  //Create a meshset
  rval = moabPtr->create_meshset(moab::MESHSET_SET,meshset);
  if(rval!=moab::MB_SUCCESS)
    mooseError("Failed to create mesh set");

  rval = createTags();
  if(rval!=moab::MB_SUCCESS)
    mooseError("Could not set up tags");

  std::map<dof_id_type,moab::EntityHandle> node_id_to_handle;
  rval = createNodes(node_id_to_handle);
  if(rval!=moab::MB_SUCCESS)
    mooseError("Could not create nodes");

  createElems(node_id_to_handle);

  // Find which elements belong to which materials
  findMaterials();
}

bool
MoabUserObject::update()
{

  TIME_SECTION(_update_timer);

  // Clear MOAB mesh data from last timestep
  reset();

  // Re-initialise the mesh data
  initMOAB();

  // Sort libMesh elements into bins of the specified variable
  if(!sortElemsByResults()) return false;

  // Find the surfaces of local temperature regions
  if(!findSurfaces()) return false;

  return true;
}

// Pass the results for named variable into the libMesh systems solution
bool
MoabUserObject::setSolution(std::string var_now,std::vector< double > &results, double scaleFactor,bool isErr,bool normToVol)
{
  TIME_SECTION(_setsolution_timer);

  // Will "throw" a mooseError if var_now not set
  // In normal run just causes a system exit, so don't catch these
  libMesh::System& sys = system(var_now);
  unsigned int iSys = sys.number();
  unsigned int iVar = sys.variable_number(var_now);

  try
    {
      setSolution(iSys,iVar,results,scaleFactor,isErr,normToVol);

      problem().copySolutionsBackwards();

      for (THREAD_ID tid = 0; tid < libMesh::n_threads(); ++tid){
        problem().getVariable(tid,var_now).computeElemValues();
      }
    }
  catch(std::runtime_error &e)
    {
      std::cerr<<e.what()<<std::endl;
      return false;
    }

  return true;
}

void
MoabUserObject::findMaterials()
{

  // Don't need to do this step if we are not performing binning
  if(!binElems) return;

  // Clear any prior data.
  mat_blocks.clear();
  initialDensities.clear();

  std::set<SubdomainID> unique_blocks;

  SubdomainID maxBlockID = mesh().n_subdomains();

  // Loop over the materials provided by the user
  for(const auto & mat : mat_names){
    // Look for the material
    std::shared_ptr< MaterialBase > mat_ptr = problem().getMaterial(mat, Moose::BLOCK_MATERIAL_DATA);

    if(mat_ptr == nullptr){
      mooseError("Could not find material "+mat );
    }

    if(binByDensity){
      // Sadly AD mats and non-AD mats do not inherit from same object
      auto openmc_den_ptr = std::dynamic_pointer_cast<OpenMCDensity>(mat_ptr);
      auto ad_openmc_den_ptr = std::dynamic_pointer_cast<ADOpenMCDensity>(mat_ptr);
      if(openmc_den_ptr == nullptr && ad_openmc_den_ptr == nullptr){
        mooseError("Please use (AD)OpenMCDensity object to represent materials with dynamic densities.");
      }
      // Retrieve initial density
      double density=0.;
      if(openmc_den_ptr != nullptr){
        density = openmc_den_ptr->origDensity();
      }
      else if(ad_openmc_den_ptr != nullptr){
        density = ad_openmc_den_ptr->origDensity();
      }
      initialDensities.push_back(density);
    }

    if(mat_ptr->numBlocks()==0){
      mooseError("Material "+mat+" does not have any blocks");
    }

    // Get the element blocks corresponding to this mat.
    std::set<SubdomainID> blocks = mat_ptr->blockIDs();

    if(!blocks.empty()){
      // Check that all the blocks are unique and have appropriate values
      unsigned int nblks_before = unique_blocks.size();
      unsigned int nblks_new = blocks.size();
      for(const auto blk: blocks){
        if(blk < 1 || blk > maxBlockID){
          std::string errmsg="Block ID "+std::to_string(blk)+" for material "+mat
            +" is inconsistent with mesh (max subdomains = "
            +std::to_string(maxBlockID)+")";
            mooseError(errmsg);
        }
        unique_blocks.insert(blk);
      }
      if(unique_blocks.size() != (nblks_before+nblks_new) ){
        mooseError("Some blocks appear in more than one material.");
      }
    }

    // Save list
    mat_blocks.push_back(blocks);
  }

  // Save number of materials
  nMatBins = mat_blocks.size();

  if(nMatBins == 0){
    mooseError("No materials were found.");
  }
  if(unique_blocks.empty()){
    mooseError("No blocks were found. Please assign at least one block to a material.");
  }

}

moab::ErrorCode
MoabUserObject::createNodes(std::map<dof_id_type,moab::EntityHandle>& node_id_to_handle)
{
  if(!hasProblem()) return moab::MB_FAILURE;

  moab::ErrorCode rval(moab::MB_SUCCESS);

  // Clear prior results.
  node_id_to_handle.clear();

  // Init array for MOAB node coords
  double 	coords[3];

  // TODO think about how the mesh is distributed...
  // Iterate over nodes in libmesh
  auto itnode = mesh().nodes_begin();
  auto endnode = mesh().nodes_end();
  for( ; itnode!=endnode; ++itnode){
    // Fetch a const ref to node
    const Node& node = **itnode;

    // Fetch coords (and scale to correct units)
    coords[0]=lengthscale*double(node(0));
    coords[1]=lengthscale*double(node(1));
    coords[2]=lengthscale*double(node(2));

    // Fetch ID
    dof_id_type id = node.id();

    // Add node to MOAB database and get handle
    moab::EntityHandle ent(0);
    rval = moabPtr->create_vertex(coords,ent);
    if(rval!=moab::MB_SUCCESS){
      node_id_to_handle.clear();
      return rval;
    }

    // Save mapping of ids.
    node_id_to_handle[id] = ent;

  }

  return rval;
}

void
MoabUserObject::createElems(std::map<dof_id_type,moab::EntityHandle>& node_id_to_handle)
{

  if(!hasProblem()){
    mooseError("No FEProblem was set in MOABUserObject");
  }

  moab::ErrorCode rval(moab::MB_SUCCESS);

  // Clear prior results.
  clearElemMaps();

  moab::Range all_elems;

  // Iterate over elements in the mesh
  auto itelem = mesh().active_elements_begin();
  auto endelem = mesh().active_elements_end();
  for( ; itelem!=endelem; ++itelem){

    // Get a reference to current elem
    Elem& elem = **itelem;

    // Get all sub-tetrahedra node sets for this element type
    ElemType type = elem.type();
    std::vector< std::vector<unsigned int> > nodeSets;
    if(!getTetSets(type,nodeSets)){
      mooseError("Could not find element (sub-)tetrahedra");
    }

    // Fetch ID
    dof_id_type id = elem.id();

    // Get the connectivity
    std::vector< dof_id_type > conn_libmesh;
    elem.connectivity(0,libMesh::IOPackage::VTK,conn_libmesh);
    if(conn_libmesh.size()!=elem.n_nodes()){
      mooseError("Element connectivity is inconsistent");
    }

    // Loop over sub tets
    for(const auto& nodeSet: nodeSets){

      if(nodeSet.size() != nNodesPerTet){
        mooseError("Wrong number of elements for sub-tetrahedron");
      }

      // Set MOAB connectivity
      std::vector<moab::EntityHandle> conn(nNodesPerTet);
      for(unsigned int iNode=0; iNode<nNodesPerTet;++iNode){

        // Get the elem node index of the ith node of the sub-tet
        unsigned int nodeIndex = nodeSet.at(iNode);

        if(nodeIndex >= conn_libmesh.size()){
          mooseError("Element index is out of range");
        }

        // Get node's entity handle
        if(node_id_to_handle.find(conn_libmesh.at(nodeIndex)) ==
           node_id_to_handle.end()){
          mooseError("Could not find node entity handle");
        }
        conn[iNode]=node_id_to_handle[conn_libmesh.at(nodeIndex)];
      }

      // Create an element in MOAB database
      moab::EntityHandle ent(0);
      rval = moabPtr->create_element(moab::MBTET,conn.data(),nNodesPerTet,ent);
      if(rval!=moab::MB_SUCCESS){
        std::string err="Could not create MOAB element: rval = "
          +std::to_string(rval);
        mooseError(err);
      }

      // Save mapping between libMesh ids and moab handles
      addElem(id,ent);

      // Save the handle for adding to entity sets
      all_elems.insert(ent);
    } // End loop over sub-tetrahedra for current elem

  } // End loop over elems

  // Add the elems to the full meshset
  rval = moabPtr->add_entities(meshset,all_elems);
  if(rval!=moab::MB_SUCCESS){
    std::string err="Could not create meshset: rval = "
      +std::to_string(rval);
    mooseError(err);
  }

  // Save the first elem
  offset = all_elems.front();

}

bool
MoabUserObject::getTetSets(ElemType type,
                           std::vector< std::vector<unsigned int> >  &perms)
{
  perms.clear();

  // Check all the elements are tets
  if(type!=TET4 && type!=TET10){
    return false;
  }

  if(type==TET4){
    perms.push_back({0,1,2,3});
  }
  else{ // TET10

    // See libmesh cell_tet10.h for vertex labelling conventions

    // One tet at each corner
    perms.push_back({0,4,6,7});
    perms.push_back({1,5,4,8});
    perms.push_back({2,6,5,9});
    perms.push_back({7,8,9,3});

    // 4 tets from central octahedron (2 back-to-back square based pyramids)
    // Central square is 4-5-9-7
    // Arbitrary choice of diagonal: 4-9
    perms.push_back({4,9,7,8});
    perms.push_back({4,5,9,8});
    perms.push_back({4,7,9,6});
    perms.push_back({4,9,5,6});
  }

  return true;
}


moab::ErrorCode
MoabUserObject::createTags()
{

  // Create some tags for later use
  moab::ErrorCode rval = moab::MB_SUCCESS;

  // First some built-in MOAB tag types
  rval = moabPtr->tag_get_handle(GEOM_DIMENSION_TAG_NAME, 1, moab::MB_TYPE_INTEGER, geometry_dimension_tag, moab::MB_TAG_DENSE | moab::MB_TAG_CREAT);
  if(rval!=moab::MB_SUCCESS) return rval;

  rval = moabPtr->tag_get_handle(GLOBAL_ID_TAG_NAME, 1, moab::MB_TYPE_INTEGER, id_tag, moab::MB_TAG_DENSE | moab::MB_TAG_CREAT);
  if(rval!=moab::MB_SUCCESS) return rval;

  rval = moabPtr->tag_get_handle(CATEGORY_TAG_NAME, CATEGORY_TAG_SIZE, moab::MB_TYPE_OPAQUE, category_tag, moab::MB_TAG_SPARSE | moab::MB_TAG_CREAT);
  if(rval!=moab::MB_SUCCESS)  return rval;

  rval = moabPtr->tag_get_handle(NAME_TAG_NAME, NAME_TAG_SIZE, moab::MB_TYPE_OPAQUE, name_tag, moab::MB_TAG_SPARSE | moab::MB_TAG_CREAT);
  if(rval!=moab::MB_SUCCESS)  return rval;

  // Some tags needed for DagMC
  rval = moabPtr->tag_get_handle("FACETING_TOL", 1, moab::MB_TYPE_DOUBLE, faceting_tol_tag,moab::MB_TAG_SPARSE | moab::MB_TAG_CREAT);
  if(rval!=moab::MB_SUCCESS)  return rval;

  rval = moabPtr->tag_get_handle("GEOMETRY_RESABS", 1, moab::MB_TYPE_DOUBLE, geometry_resabs_tag, moab::MB_TAG_SPARSE | moab::MB_TAG_CREAT);
  if(rval!=moab::MB_SUCCESS)  return rval;

  // Set the values for DagMC faceting / geometry tolerance tags on the mesh entity set
  rval = moabPtr->tag_set_data(faceting_tol_tag, &meshset, 1, &faceting_tol);
  if(rval!=moab::MB_SUCCESS)  return rval;

  rval = moabPtr->tag_set_data(geometry_resabs_tag, &meshset, 1, &geom_tol);
  return rval;
}

moab::ErrorCode
MoabUserObject::createGroup(unsigned int id, std::string name,moab::EntityHandle& group_set)
{
  // Create a new mesh set
  moab::ErrorCode rval = moabPtr->create_meshset(moab::MESHSET_SET,group_set);
  if(rval!=moab::MB_SUCCESS) return rval;

  // Set the tags for this material
  return setTags(group_set,name,"Group",id,4);
}


moab::ErrorCode
MoabUserObject::createVol(unsigned int id,moab::EntityHandle& volume_set,moab::EntityHandle group_set)
{
  moab::ErrorCode rval = moabPtr->create_meshset(moab::MESHSET_SET,volume_set);
  if(rval!=moab::MB_SUCCESS) return rval;

  rval =  setTags(volume_set,"","Volume",id,3);
  if(rval != moab::MB_SUCCESS) return rval;

  // Add the volume to group
  rval = moabPtr->add_entities(group_set, &volume_set,1);
  if(rval != moab::MB_SUCCESS) return rval;

  return rval;
}

moab::ErrorCode
MoabUserObject::createSurf(unsigned int id,moab::EntityHandle& surface_set, moab::Range& faces,  std::vector<VolData> & voldata)
{
  // Create meshset
  moab::ErrorCode rval = moabPtr->create_meshset(moab::MESHSET_SET,surface_set);
  if(rval!=moab::MB_SUCCESS) return rval;

  // Set tags
  rval = setTags(surface_set,"","Surface",id,2);
  if(rval!=moab::MB_SUCCESS) return rval;

  // Add tris to the surface
  rval = moabPtr->add_entities(surface_set,faces);
  if(rval != moab::MB_SUCCESS) return rval;

  // Create entry in map
  surfsToVols[surface_set] = std::vector<VolData>();

  // Add volume to list associated with this surface
  for(const auto & data : voldata){
    rval = updateSurfData(surface_set,data);
    if(rval != moab::MB_SUCCESS) return rval;
  }

  return moab::MB_SUCCESS;
}

moab::ErrorCode
MoabUserObject::updateSurfData(moab::EntityHandle surface_set,VolData data)
{

  // Add the surface to the volume set
  moab::ErrorCode rval = moabPtr->add_parent_child(data.vol,surface_set);
  if(rval != moab::MB_SUCCESS) return rval;

  // Set the surfaces sense
  rval = gtt->set_sense(surface_set,data.vol,int(data.sense));
  if(rval != moab::MB_SUCCESS) return rval;

  // Save
  surfsToVols[surface_set].push_back(data);

  return moab::MB_SUCCESS;
}


moab::ErrorCode
MoabUserObject::setTags(moab::EntityHandle ent, std::string name, std::string category, unsigned int id, int dim)
{

  moab::ErrorCode rval;

  // Set the name tag
  if(name!=""){
    rval = setTagData(name_tag,ent,name,NAME_TAG_SIZE);
    if(rval!=moab::MB_SUCCESS) return rval;
  }

  // Set the category tag
  if(category!=""){
    rval = setTagData(category_tag,ent,category,CATEGORY_TAG_SIZE);
    if(rval!=moab::MB_SUCCESS) return rval;
  }

  // Set the dimension tag
  rval = setTagData(geometry_dimension_tag,ent,&dim);
  if(rval!=moab::MB_SUCCESS) return rval;

  // Set the id tag
  rval = setTagData(id_tag,ent,&id);
  return rval;

}

moab::ErrorCode
MoabUserObject::setTagData(moab::Tag tag, moab::EntityHandle ent, std::string data, unsigned int SIZE)
{
  auto namebuf= new char[SIZE];
  memset(namebuf,'\0', SIZE); // fill C char array with null
  strncpy(namebuf,data.c_str(),SIZE-1);
  moab::ErrorCode rval = moabPtr->tag_set_data(tag,&ent,1,namebuf);
  // deallocate memory
  delete[] namebuf;
  return rval;
}

moab::ErrorCode
MoabUserObject::setTagData(moab::Tag tag, moab::EntityHandle ent, void* data)
{
  return moabPtr->tag_set_data(tag,&ent,1,data);
}

void
MoabUserObject::clearElemMaps()
{
  _id_to_elem_handles.clear();
  offset=0;
}

void
MoabUserObject::addElem(dof_id_type id,moab::EntityHandle ent)
{
  if(_id_to_elem_handles.find(id)==_id_to_elem_handles.end()){
    _id_to_elem_handles[id]=std::vector<moab::EntityHandle>();
  }
  (_id_to_elem_handles[id]).push_back(ent);
}

void
MoabUserObject::setSolution(unsigned int iSysNow,  unsigned int iVarNow, std::vector< double > &results, double scaleFactor, bool isErr, bool normToVol)
{

  if(!hasProblem())
    mooseError("FE problem was not set");

  // Fetch a reference to our system
  libMesh::System& sys = systems().get_system(iSysNow);

  // Keep track of whether we have non-trivial results on this processor.
  bool procHasNonZeroResult=false;

  // When we set the solution, we only want to set dofs that belong to this process
  auto itelem  = mesh().active_local_elements_begin();
  auto endelem = mesh().active_local_elements_end();
  for( ; itelem!=endelem; ++itelem){

    Elem& elem = **itelem;
    dof_id_type id = elem.id();

    // Convert the elem id to a list of entity handles
    if(_id_to_elem_handles.find(id)==_id_to_elem_handles.end())
      throw std::runtime_error("Elem id not matched to an entity handle");
    std::vector<moab::EntityHandle> ents =  _id_to_elem_handles[id];

    // Sum over the result bins for this elem
    double result=0.;
    for(const auto ent : ents){
      // Conversion to bin index
      unsigned int binIndex = ent - offset;

      if( (binIndex+1) > results.size() ){
        throw std::runtime_error("Mismatch in size of results vector and number of elements");
      }

      result += results.at(binIndex);
    }

    if(isErr){
      // result is a [sum of] variance[s]: sqrt to get the error.
      // NB: for second order mesh this is equivalent to summing errors in quadrature,
      // so won't quite be equivalent to the variance of the mean on the original mesh,
      // but difference should be small for large sample size.
      result=sqrt(result);
    }

    // Scale the result
    result *= scaleFactor;

    if(normToVol){
      // Fetch the volume of element
      double vol = elem.volume();
      // Normalise result to the element volume
      result /= vol;
    }

    // Get the solution index for this element
    dof_id_type index = elem_to_soln_index(elem,iSysNow,iVarNow);

    // Set the solution for this index
    sys.solution->set(index,result);

    if(!procHasNonZeroResult && fabs(result) > 1.e-9){
      procHasNonZeroResult=true;
    }
  }

  // Synchronise processes
  comm().barrier();

  // If we found a non-zero result on this process, tell all the other proceses
  // Convert to int, and find maximum accross all procs
  int hasNonZeroResultInt=int(procHasNonZeroResult);
  comm().max(hasNonZeroResultInt);
  // Convert back to bool
  bool hasNonZeroResultGlobal = bool(hasNonZeroResultInt);

  // Warn if there was no non-zero result accross all processes
  if(!hasNonZeroResultGlobal){
    mooseWarning("OpenMC results are everywhere zero.");
  }

  sys.solution->close();

}

void MoabUserObject::getMaterialProperties(std::vector<std::string>& mat_names_out,
                                           std::vector<double>& initial_densities,
                                           std::vector<std::string>& tails,
                                           std::vector<MOABMaterialProperties>& properties)
{
  // We shouldn't be calling this if we didn't provide any materials
  if(openmc_mat_names.empty())
    mooseError("No material names were provided.");

  // Set the list of materials names we expect to find in openmc
  mat_names_out=openmc_mat_names;

  // We shouldn't call this function until after initMOAB
  // due to timing of intialisation of MOOSE materials)
  if(binByDensity && initialDensities.size()!=openmc_mat_names.size())
    mooseError("Initial densities not yet initialised.");

  // Set the original densities of materials
  initial_densities = initialDensities;
  // Convert to g/cm3 if not already in these units
  if(densityscale!= 1.0){
    for(size_t iMat=0; iMat<initial_densities.size(); iMat++){
      initial_densities.at(iMat) *= densityscale;
    }
  }

  tails.clear();
  properties.clear();

  // Loop over density bins
  for(unsigned int iDen=0; iDen<nDenBins; iDen++){

    // Retrieve the relative density
    double rel_den = den_midpoints.at(iDen);

    // Loop over temperature bins
    for(unsigned int iVar=0; iVar<nVarBins; iVar++){
      // Retrieve the average bin temperature
      double temp = midpoints.at(iVar);

      // Get the name modifier
      int iNewMatBin = getMatBin(iVar,iDen);
      std::string tail ="_"+std::to_string(iNewMatBin);
      tails.push_back(tail);

      // Save material properties
      MOABMaterialProperties mat_props;
      mat_props.temp = temp;
      mat_props.rel_density = rel_den;
      properties.push_back(mat_props);
    }
  }

}


dof_id_type
MoabUserObject::elem_to_soln_index(const Elem& elem,unsigned int iSysNow,  unsigned int iVarNow)
{
    // Expect only one component, but check anyay
  unsigned int n_components = elem.n_comp(iSysNow,iVarNow);
  if(n_components != 1){
    throw std::runtime_error("Unexpected number of expected solution components");
  }

  // Get the degree of freedom number
  dof_id_type soln_index = elem.dof_number(iSysNow,iVarNow,0);

  return soln_index;
}

void
MoabUserObject::initBinningData(){
  // Don't attempt to bin results if we haven't been provided with a variable
  if(!binElems) return;

  // Create mesh functions for each variable we are bining by
  setMeshFunction(var_name);
  if(binByDensity){
    setMeshFunction(den_var_name);
  }

}

NumericVector<Number>&
MoabUserObject::getSerialisedSolution(libMesh::System* sysPtr)
{
  if(sysPtr==nullptr) mooseError("System pointer is null");

  // Get the index of this system
  unsigned int iSysNow = sysPtr->number();

  // Look up if we already have the serial solution
  if(serial_solutions.find(iSysNow)==serial_solutions.end()){
    // Initialize the serial solution vector
    // N.B. For big problems this is going to be a memory bottleneck
    auto serial_solution = NumericVector<Number>::build(comm());
    serial_solution->init(sysPtr->n_dofs(), false, SERIAL);

    // Pull down a full copy of this vector on every processor
    sysPtr->solution->localize(*serial_solution);

    // Move the unique pointer to the map
    serial_solutions[iSysNow] = std::move(serial_solution);
  }

  // Return a reference
  NumericVector<Number>& solution_ref = *(serial_solutions[iSysNow]);
  return solution_ref;
}

void
MoabUserObject::setMeshFunction(std::string var_name_in)
{

  libMesh::System* sysPtr;
  unsigned int iVarNow;

  // Get the system and variable number
  try{
    sysPtr = &system(var_name_in);
    iVarNow = sysPtr->variable_number(var_name_in);
  }
  catch(std::exception &e){
    mooseError(e.what());
  }

  std::vector<unsigned int> var_nums(1,iVarNow);

  // Fetch the serialised solution for this system
  NumericVector<Number>& serial_solution = getSerialisedSolution(sysPtr);

  // Create the mesh function
  meshFunctionPtrs[var_name_in] =
    std::make_shared<MeshFunction>(systems(),
                                   serial_solution,
                                   sysPtr->get_dof_map(),
                                   var_nums);

  // Initialise mesh function
  meshFunctionPtrs[var_name_in]->init(Trees::BuildType::ELEMENTS);
  meshFunctionPtrs[var_name_in]->enable_out_of_mesh_mode(-1.0);

}

double
MoabUserObject::evalMeshFunction(std::shared_ptr<MeshFunction> meshFunctionPtr,
                                 const Point& p)
{

  // Evaluate the mesh function on this point
  double result = double((*meshFunctionPtr)(p));

  if(result<0.){
    // If we got a negative result, understand why
    const PointLocatorBase& locator = meshFunctionPtr->get_point_locator();
    const Elem * elemPtr = locator(p);
    if(elemPtr == nullptr){
      mooseError("Point is out of mesh");
          }
    else{
      mooseError("Negative result found in solution vector");
    }
  }

  return result;

}

std::shared_ptr<MeshFunction>
MoabUserObject::getMeshFunction(std::string var_name_in)
{
  if(meshFunctionPtrs.find(var_name_in) ==meshFunctionPtrs.end()){
    std::string err;
    err="No mesh function initialised for variable "+var_name_in;
    mooseError(err);
  }
  if(meshFunctionPtrs[var_name_in]==nullptr){
    std::string err;
    err="Mesh function ptr for variable "+var_name_in + "is null";
    mooseError(err);
  }
  return meshFunctionPtrs[var_name_in];
}


bool
MoabUserObject::sortElemsByResults()
{
  // Don't attempt to bin results if we haven't been provided with a variable
  if(!binElems) return false;

   // Clear any prior data;
  resetContainers();

  // Get the mesh functions for temperature and densities
  std::shared_ptr<MeshFunction> meshFunctionPtr = getMeshFunction(var_name);
  std::shared_ptr<MeshFunction> denMeshFunctionPtr(nullptr);
  if(binByDensity){
    denMeshFunctionPtr= getMeshFunction(den_var_name);
  }

  // Outer loop over materials
  for(unsigned int iMat=0; iMat<nMatBins; iMat++){

    // Get the subdomains for this material
    std::set<SubdomainID>& blocks = mat_blocks.at(iMat);

    // Loop over subdomains
    for( const auto block_id : blocks){

      // Iterate over elements in this material whose dofs belong to this proc
      auto itelem = mesh().active_local_subdomain_elements_begin(block_id);
      auto endelem = mesh().active_local_subdomain_elements_end(block_id);
      for( ; itelem!=endelem; ++itelem){

        Elem& elem = **itelem;
        dof_id_type id = elem.id();

        // Fetch the central point of this element
        Point p = elemCentroid(elem);

        int iDenBin=0;
        if(binByDensity){
          // Evaluate the density mesh function on this point
          double den_result = evalMeshFunction(denMeshFunctionPtr,p);
          // Get the initial density for this material
          double initial_den = initialDensities.at(iMat);
          // Get the relative difference in density
          double rel_den = den_result/initial_den - 1.0;
          // Get the relative density bin number
          iDenBin = getRelDensityBin(rel_den);
        }

        // Evaluate the temp mesh function on this point
        double temp_result = evalMeshFunction(meshFunctionPtr,p);

        // Calculate the bin number for this value
        int iBin = getResultsBin(temp_result);

        // Sort elem into a bin
        int iSortBin = getSortBin(iBin,iDenBin,iMat);
        sortedElems.at(iSortBin).insert(id);

      }
    }
  }

  // Wait for all processes to finish
  comm().barrier();

  // MPI communication
  for( unsigned int iSortBin=0; iSortBin< sortedElems.size(); iSortBin++){
    // Get a reference
    std::set<dof_id_type>& sortedBinElems = sortedElems.at(iSortBin);
    // Get the union of the set over all procs
    communicateDofSet(sortedBinElems);
  }

  // Check everything adds up
  size_t elemCountCheck=0;
  for(const auto & elemSet : sortedElems){
    elemCountCheck += elemSet.size();
  }

  if(elemCountCheck != mesh().n_active_elem()){
    mooseError("Disparity in number of sorted elements.");
  }

  return true;

}

Point
MoabUserObject::elemCentroid(Elem& elem){
  Point centroid(0.,0.,0.);
  unsigned int nNodes = elem.n_nodes();
  for(unsigned int iNode=0; iNode<nNodes; ++iNode){
    // Get the point coords for this node
    const Point& point = elem.point(iNode);
    centroid += point;
  }
  centroid /= double(nNodes);
  return centroid;
}

void
MoabUserObject::communicateDofSet(std::set<dof_id_type>& dofset)
{
  comm().set_union(dofset);
}

bool
MoabUserObject::findSurfaces()
{

  moab::ErrorCode rval = moab::MB_SUCCESS;
  try{
    // Find all neighbours in mesh
    mesh().find_neighbors();

    // Counter for volumes
    unsigned int vol_id=0;

    // Counter for surfaces
    unsigned int surf_id=0;

    // Loop over material bins
    for(unsigned int iMat=0; iMat<nMatBins; iMat++){

      // Get the base material name:
      std::string mat_name = "mat:"+openmc_mat_names.at(iMat);

      // Loop over density bins
      for(unsigned int iDen=0; iDen<nDenBins; iDen++){

        // Loop over temperature bins
        for(unsigned int iVar=0; iVar<nVarBins; iVar++){

          // Update material name
          std::string updated_mat_name=mat_name;
          int iNewMatBin = getMatBin(iVar,iDen);
          updated_mat_name+="_"+std::to_string(iNewMatBin);

          // Create a material group
          // Todo set temp in metadata?
          int iSortBin = getSortBin(iVar,iDen,iMat);
          moab::EntityHandle group_set;
          unsigned int group_id = iSortBin+1;
          rval = createGroup(group_id,updated_mat_name,group_set);
          if(rval != moab::MB_SUCCESS) return false;

          // Sort elems in this mat-density-temp bin into local regions
          std::vector<moab::Range> regions;
          groupLocalElems(sortedElems.at(iSortBin),regions);

          // Loop over all regions and find surfaces
          for(const auto & region : regions){
            moab::EntityHandle volume_set;
            if(!findSurface(region,group_set,vol_id,surf_id,volume_set)){
              return false;
            }

          } // End loop over local regions

        } // End loop over temperature bins
      } // End loop over density bins
    } // End loop over materials

    // Finally, build a graveyard
    rval = buildGraveyard(vol_id,surf_id);
    if(rval != moab::MB_SUCCESS) return false;

  }
  catch(std::exception &e){
    std::cerr<<e.what()<<std::endl;
    return false;
  }

  // Optionally write to file
  if(output_skins || output_full){
    if(!write()) return false;
  }

  return true;
}

bool
MoabUserObject::write()
{

  // Only write to file on root process
  if(processor_id() != 0) return true;

  if(n_write >= n_output){
    output_skins=false; // Don't write any more times
    output_full=false; // Don't write any more times
    return true;
  }

  if( (n_its % n_period) == 0 ){

    if(output_skins){
      // Generate list of surfaces to write.
      std::vector<moab::EntityHandle> surfs;
      for( auto itsurf : surfsToVols){
        surfs.push_back(itsurf.first);
      }
      std::string filename = output_base + "_" + std::to_string(n_write) +".h5m";
      std::cout << "Writing MOAB surface mesh to "<< filename << std::endl;
      moab::ErrorCode rval = moabPtr->write_mesh(filename.c_str(),surfs.data(),surfs.size());
      if(rval != moab::MB_SUCCESS) return false;
    }
    if(output_full){
      std::string filename = output_base_full + "_" + std::to_string(n_write) +".h5m";
      std::cout << "Writing MOAB mesh to "<< filename << std::endl;
      moab::ErrorCode rval = moabPtr->write_mesh(filename.c_str());
      if(rval != moab::MB_SUCCESS) return false;
    }
    n_write++;
  }

  n_its++;
  return true;
}

void
MoabUserObject::groupLocalElems(std::set<dof_id_type> elems, std::vector<moab::Range>& localElems)
{
  while(!elems.empty()){

    // Create a new local range of moab handles
    moab::Range local;

    // Retrieve and remove the fisrt elem
    auto it = elems.begin();
    dof_id_type next = *it;
    elems.erase(it);

    std::set<dof_id_type> neighbors;
    neighbors.insert(next);

    while(!neighbors.empty()){

      std::set<dof_id_type> new_neighbors;

      // Loop over all the new neighbors
      for(auto& next : neighbors){

        // Get the MOAB handles, and add to local set
        // (May be more than one if this libMesh elem has sub-tetrahedra)
        if(_id_to_elem_handles.find(next)==_id_to_elem_handles.end()){
          mooseError("No entity handles found for libmesh id.");
        }
        std::vector<moab::EntityHandle> ents = _id_to_elem_handles[next];
        for(const auto ent : ents){
          local.insert(ent);
        }

        // Get the libMesh element
        Elem& elem = mesh().elem_ref(next);

        // How many nearest neighbors (general element)?
        unsigned int NN = elem.n_neighbors();

        // Loop over neighbors
        for(unsigned int i=0; i<NN; i++){

          const Elem * nnptr = elem.neighbor_ptr(i);
          // If on boundary, some may be null ptrs
          if(nnptr == nullptr) continue;

          dof_id_type idnn = nnptr->id();

          // Select only those that are in the current bin
          if(elems.find(idnn)!= elems.end()){
            new_neighbors.insert(idnn);
            // Remove from those still available
            elems.erase(idnn);
          }

        }// End loop over new neighbors

      }// End loop over previous neighbors

      // Found all the new neighbors, done with current set.
      neighbors = new_neighbors;

    }
    // Done, no more local neighbors in the current bin.

    // Save this moab range of local neighbors
    localElems.push_back(local);
  }
  // Done, assigned all elems in bin to a local range.
 }

void
MoabUserObject::resetContainers()
{
  unsigned int nSortBins = nMatBins*nDenBins*nVarBins;
  sortedElems.clear();
  sortedElems.resize(nSortBins);

  // Update the serial solutions
  for(const auto& sol :  serial_solutions){
    System & sys = 	systems().get_system(sol.first);

    // Check if solution vector size has changed, e.g. due to mesh refinement
    if(sys.n_dofs() != sol.second->size()){
      // clear
      sol.second->init(0,false,SERIAL);
      // resize
      sol.second->init(sys.n_dofs(),false,SERIAL);
    }

    sys.solution->localize(*sol.second);
  }
}

void
MoabUserObject::reset()
{
  // Clear data
  moabPtr.reset(new moab::Core());

  // Create a skinner and geometry topo tool
  skinner.reset(new moab::Skinner(moabPtr.get()));
  gtt.reset(new moab::GeomTopoTool(moabPtr.get()));

  // Clear entity set maps
  surfsToVols.clear();
}

int
MoabUserObject::getResultsBin(double value)
{
  if(logscale) return getResultsBinLog(value);
  else return getResultsBinLin(value);
}

inline int
MoabUserObject::getResultsBinLin(double value)
{
  return int(floor((value-var_min)/bin_width));
}

int
MoabUserObject::getResultsBinLog(double value)
{

  // Get the power of 10
  double powFloat = log10(value);

  // Round down power to nearest int
  double powMajor = floor(powFloat);

  // Get the major  bin
  int iMajor = int(powMajor)-powMin;

  // Get the minor bin
  int iMinor = int(floor( (powFloat - powMajor)*double(nMinor) ));

  // Get bin - can be out of range if results are not inside the limits specified by user
  int iBin = nMinor*iMajor + iMinor;

  return iBin;
}

void
MoabUserObject::calcMidpoints()
{
  if(logscale) calcMidpointsLog();
  else calcMidpointsLin();
}


void
MoabUserObject::calcMidpointsLin()
{
  calcMidpointsLin(var_min,bin_width,nVarBins,midpoints);
}

void
MoabUserObject::calcMidpointsLog()
{
  double powDiff = 1./double(nMinor);
  double powStart = double(powMin) - 0.5*powDiff;
  double var_now = pow(10,powStart);
  double prodDiff = pow(10,powDiff);
  for(unsigned int iVar=0; iVar<nVarBins; iVar++){
    var_now *= prodDiff;
    midpoints.push_back(var_now);
  }
}

inline int
MoabUserObject::getRelDensityBin(double value)
{
  return int(floor((value-rel_den_min)/rel_den_bw));
}

int
MoabUserObject::getSortBin(int iVarBin, int iDenBin, int iMat,int nVarBinsIn, int nDenBinsIn,int nMatsIn)
{

  if(iMat<0 || iMat >= nMatsIn ){
    std::string err = "Material index is out of range";
    mooseError(err);
  }
  if(iDenBin<0 || iDenBin >= nDenBinsIn ){
    std::string err = "Relative density of material "+
      mat_names.at(iMat)+" fell outside of binning range";
    mooseError(err);
  }
  if(iVarBin<0 || iVarBin >= nVarBinsIn ){
    std::string err = "Relative temperature of material "+
      mat_names.at(iMat)+" fell outside of binning range";
    mooseError(err);
  }

  int nSortBins = nMatsIn*nDenBinsIn*nVarBins;
  int iSortBin= nVarBinsIn*(nDenBinsIn*iMat + iDenBin) + iVarBin;
  if(iSortBin<0 || iSortBin >= nSortBins){
    mooseError("Cannot find bin index.");
  }
  return iSortBin;
}

int
MoabUserObject::getMatBin(int iVarBin, int iDenBin, int nVarBinsIn, int nDenBinsIn)
{

  if(iDenBin<0 || iDenBin >= nDenBinsIn ){
    std::string err = "Relative density of material fell outside of binning range";
    mooseError(err);
  }
  if(iVarBin<0 || iVarBin >= nVarBinsIn ){
    std::string err = "Relative temperature of material fell outside of binning range";
    mooseError(err);
  }

  int nMatBins = nDenBinsIn*nVarBins;
  int iMatBin= nVarBinsIn*iDenBin + iVarBin;
  if(iMatBin<0 || iMatBin >= nMatBins){
    mooseError("Cannot find material bin index.");
  }
  return iMatBin;
}

void
MoabUserObject::calcDenMidpoints()
{
  rel_den_bw = (rel_den_max-rel_den_min)/double(nDenBins);
  calcMidpointsLin(rel_den_min,rel_den_bw,nDenBins,den_midpoints);
}

void
MoabUserObject::calcMidpointsLin(double var_min_in, double bin_width_in,int nbins_in,std::vector<double>& midpoints_in)
{
  double var_now = var_min_in - bin_width_in/2.0;
  for(unsigned int iVar=0; iVar<nbins_in; iVar++){
    var_now += bin_width_in;
    midpoints_in.push_back(var_now);
  }
}

bool
MoabUserObject::findSurface(const moab::Range& region,moab::EntityHandle group, unsigned int & vol_id, unsigned int & surf_id,moab::EntityHandle& volume_set)
{

  moab::ErrorCode rval;

  // Create a volume set
  vol_id++;
  rval = createVol(vol_id,volume_set,group);
  if(rval != moab::MB_SUCCESS) return false;

  // Find surfaces from these regions
  moab::Range tris; // The tris of the surfaces
  moab::Range rtris;  // The tris which are reversed with respect to their surfaces
  rval = skinner->find_skin(0,region,false,tris,&rtris);
  if(rval != moab::MB_SUCCESS) return false;
  if(tris.size()==0 && rtris.size()==0) return false;


  // Create surface sets for the forwards tris
  VolData vdata = {volume_set,Sense::FORWARDS};
  rval = createSurfaces(tris,vdata,surf_id);
  if(rval != moab::MB_SUCCESS) return false;

  // Create surface sets for the reversed tris
  vdata.sense =Sense::BACKWARDS;
  rval = createSurfaces(rtris,vdata,surf_id);
  if(rval != moab::MB_SUCCESS) return false;

  return true;
}


moab::ErrorCode
MoabUserObject::createSurfaces(moab::Range& faces, VolData& voldata, unsigned int& surf_id){

  moab::ErrorCode rval = moab::MB_SUCCESS;

  if(faces.empty()) return rval;

  // Loop over the surfaces we have already created
  for ( const auto & surfpair : surfsToVols ) {

    // Local copies of surf/vols
    moab::EntityHandle surf = surfpair.first;
    std::vector<VolData> vols = surfpair.second;

    // First get the entities in this surface
    moab::Range tris;
    rval = moabPtr->get_entities_by_handle(surf,tris);
    if(rval!=moab::MB_SUCCESS) return rval;

    // Find any tris that live in both surfs
    moab::Range overlap = moab::intersect(tris,faces);
    if(!overlap.empty()) {

      // Check if the tris are a subset or the entire surf
      if(tris.size()==overlap.size()){
        // Whole surface -> Just need to update the volume relationships
        rval = updateSurfData(surf,voldata);
      }
      else{
        // If overlap is subset, subtract shared tris from this surface and create a new shared surface
        rval = moabPtr->remove_entities(surf,overlap);
        if(rval!=moab::MB_SUCCESS) return rval;

        // Append our new volume to the list that share this surf
        vols.push_back(voldata);

        // Create a new shared surface
        moab::EntityHandle shared_surf;
        surf_id++;
        rval = createSurf(surf_id,shared_surf,overlap,vols);
        if(rval!=moab::MB_SUCCESS) return rval;
      }

      // Subtract from the input list
      for( auto& shared : overlap ){
        faces.erase(shared);
      }
      if(faces.empty()) break;
    }
  }

  if(!faces.empty()){
    moab::EntityHandle surface_set;
    std::vector<VolData> voldatavec(1,voldata);
    surf_id++;
    rval = createSurf(surf_id,surface_set,faces,voldatavec);
    if(rval != moab::MB_SUCCESS) return rval;
  }

  return rval;
}

moab::ErrorCode MoabUserObject::buildGraveyard( unsigned int & vol_id, unsigned int & surf_id)
{
  moab::ErrorCode rval(moab::MB_SUCCESS);

  // Create the graveyard set
  moab::EntityHandle graveyard;
  unsigned int id = nMatBins*nVarBins*nDenBins+1;
  std::string mat_name = "mat:Graveyard";
  rval = createGroup(id,mat_name,graveyard);
  if(rval != moab::MB_SUCCESS) return rval;

  // Create a volume set
  moab::EntityHandle volume_set;
  vol_id++;
  rval = createVol(vol_id,volume_set,graveyard);
  if(rval != moab::MB_SUCCESS) return rval;

  // Set up for the volume data to pass to surfs
  VolData vdata = {volume_set,Sense::FORWARDS};

  // Find a bounding box
  BoundingBox bbox =  MeshTools::create_bounding_box(mesh());

  // Create inner surface with normals pointing into of box
  rval = createSurfaceFromBox(bbox,vdata,surf_id,false,scalefactor_inner);
  if(rval != moab::MB_SUCCESS) return rval;

  // Create outer surface with face normals pointing out of the box
  rval = createSurfaceFromBox(bbox,vdata,surf_id,true,scalefactor_outer);
  return rval;
}

moab::ErrorCode
MoabUserObject::createSurfaceFromBox(const BoundingBox& box, VolData& voldata, unsigned int& surf_id, bool normalout, double factor)
{
  // Create the vertices of the box
  std::vector<moab::EntityHandle> vert_handles;
  moab::ErrorCode rval = createNodesFromBox(box,factor,vert_handles);
  if(rval!=moab::MB_SUCCESS) return rval;
  else if(vert_handles.size() != 8) mooseError("Failed to get box coords");

  // Create the tris in 4 groups of 3 (4 open tetrahedra)
  moab::Range tris;
  rval = createCornerTris(vert_handles,0,1,2,4,normalout,tris);
  if(rval!=moab::MB_SUCCESS) return rval;

  rval = createCornerTris(vert_handles,3,2,1,7,normalout,tris);
  if(rval!=moab::MB_SUCCESS) return rval;

  rval = createCornerTris(vert_handles,6,4,2,7,normalout,tris);
  if(rval!=moab::MB_SUCCESS) return rval;

  rval = createCornerTris(vert_handles,5,1,4,7,normalout,tris);
  if(rval!=moab::MB_SUCCESS) return rval;

  moab::EntityHandle surface_set;
  std::vector<VolData> voldatavec(1,voldata);
  surf_id++;
  return createSurf(surf_id,surface_set,tris,voldatavec);
}

moab::ErrorCode
MoabUserObject::createNodesFromBox(const BoundingBox& box,double factor,std::vector<moab::EntityHandle>& vert_handles)
{
  moab::ErrorCode rval(moab::MB_SUCCESS);

  // Fetch the vertices of the box
  std::vector<Point> verts = boxCoords(box,factor);
  if(verts.size() != 8) mooseError("Failed to get box coords");

  // Array to represent a coord in moab
  double coord[3];
  // Create the vertices in moab and get the handles
  for(const auto & vert : verts){
    coord[0]=vert(0);
    coord[1]=vert(1);
    coord[2]=vert(2);

    moab::EntityHandle ent;
    rval = moabPtr->create_vertex(coord,ent);
    if(rval!=moab::MB_SUCCESS){
      vert_handles.clear();
      return rval;
    }
    vert_handles.push_back(ent);
  }
  return rval;
}

std::vector<Point>
MoabUserObject::boxCoords(const BoundingBox& box, double factor)
{
  Point minpoint = (box.min())*lengthscale;
  Point maxpoint = (box.max())*lengthscale;
  Point diff = (maxpoint - minpoint)/2.0;
  Point origin = minpoint + diff;

  // Rescale sidelength of box by specified factor
  diff *=factor;

  // modify minpoint
  minpoint = origin - diff;

  // Vectors for sides of box
  Point dx(2.0*diff(0),0.,0.);
  Point dy(0.,2.0*diff(1),0.);
  Point dz(0.,0.,2.0*diff(2));

  // Start at (-,-,-) and add side vectors
  std::vector<Point> verts(8,minpoint);
  for(unsigned int idz=0; idz<2; idz++){
    for(unsigned int idy=0; idy<2; idy++){
      for(unsigned int idx=0; idx<2; idx++){
        unsigned int ibin = 4*idz + 2*idy+ idx;
        verts.at(ibin) += double(idx)*dx + double(idy)*dy + double(idz)*dz;
      }
    }
  }

  return verts;
}

moab::ErrorCode
MoabUserObject::createCornerTris(const std::vector<moab::EntityHandle> & verts,
                                 unsigned int corner,
                                 unsigned int v1, unsigned int v2 ,unsigned int v3,
                                 bool normalout, moab::Range &surface_tris)
{
  // Create 3 tris stemming from one corner (i.e. an open tetrahedron)
  // Assume first is the central corner, and the others are labelled clockwise looking down on the corner
  moab::ErrorCode rval = moab::MB_SUCCESS;
  unsigned int indices[3] = {v1,v2,v3};

  //Create each tri by a cyclic permutation of indices
  for(unsigned int i=0; i<3; i++){
    // v1,v2 = 0,1; 1,2; 2;0
    int v1 = indices[i%3];
    int v2 = indices[(i+1)%3];
    if(normalout){
      // anti-clockwise: normal points outwards
      rval = createTri(verts,corner,v2,v1,surface_tris);
    }
    else{
      // clockwise: normal points inwards
      rval = createTri(verts,corner,v1,v2,surface_tris);
    }
    if(rval!=moab::MB_SUCCESS) return rval;
  }
  return rval;
}

moab::ErrorCode
MoabUserObject::createTri(const std::vector<moab::EntityHandle> & vertices,unsigned int v1, unsigned int v2 ,unsigned int v3, moab::Range &surface_tris) {
  moab::ErrorCode rval = moab::MB_SUCCESS;
  moab::EntityHandle triangle;
  moab::EntityHandle connectivity[3] = { vertices[v1],vertices[v2],vertices[v3] };
  rval = moabPtr->create_element(moab::MBTRI,connectivity,3,triangle);
  surface_tris.insert(triangle);
  return rval;
}

#endif
