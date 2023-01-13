#pragma once

// MOOSE includes
#include "UserObject.h"
#include "MaterialBase.h"

// MOAB includes
#include "moab/Core.hpp"
#include "moab/Skinner.hpp"
#include "moab/GeomTopoTool.hpp"
#include "MBTagConventions.hpp"

// Libmesh includes
#include <libmesh/elem.h>
#include <libmesh/enum_io_package.h>
#include <libmesh/enum_order.h>
#include <libmesh/enum_fe_family.h>
#include <libmesh/equation_systems.h>
#include <libmesh/system.h>
#include <libmesh/mesh_tools.h>
#include <libmesh/mesh_function.h>

/// Convenience struct
struct MOABMaterialProperties{
  double rel_density;
  double temp;
};


// Forward Declarations
class MoabUserObject;


/**
    \brief UserObject class which wraps a moab::Interface pointer.

    The main role of this class is to bin elements of the mesh
    according named variables (presumed temperature and density),
    and subsequently perform a skinning operation to find the
    surfaces of these local regions.
 */
class MoabUserObject : public UserObject
{
 public:

  MoabUserObject(const InputParameters & parameters);

  static InputParameters validParams();

  /// Override MOOSE virtual method to do nothing
  virtual void execute(){};
  /// Override MOOSE virtual method to do nothing
  virtual void initialize(){};
  /// Override MOOSE virtual method to do nothing
  virtual void finalize(){};
  /// Override MOOSE virtual method to do nothing
  virtual void threadJoin(const UserObject & /*uo*/){};

  /// Pass in the FE Problem
  void setProblem(FEProblemBase * problem) {
    if(problem==nullptr)
      throw std::logic_error("Problem passed is null");
    _problem_ptr = problem;
  } ;

  /// Check if problem has been set
  bool hasProblem(){ return !( _problem_ptr == nullptr ); };

  /// Get a reference to the FE problem
  FEProblemBase& problem();

  /// Initialise MOAB
  void initMOAB();

  /// Intialise objects needed to perform binning of elements
  void initBinningData();

  /// Clear mesh data
  void reset();

  /// Update MOAB with any results from MOOSE
  bool update();

  /// Pass the OpenMC results into the libMesh systems solution
  bool setSolution(std::string var_now,std::vector< double > &results, double scaleFactor=1., bool isErr=false, bool normToVol=true);

  /// Retrieve a list of original material names and properties
  void getMaterialProperties(std::vector<std::string>& mat_names_out,
                             std::vector<double>& initial_densities,
                             std::vector<std::string>& tails,
                             std::vector<MOABMaterialProperties>& properties);

  /// Publically available pointer to MOAB interface
  std::shared_ptr<moab::Interface> moabPtr;

private:

  // Private types

  /// Encode the whether the surface normal faces into or out of the volume
  enum Sense { BACKWARDS=-1, FORWARDS=1};

  /// \brief Encode MOAB information about volumes needed when creating surfaces
  struct VolData{
    moab::EntityHandle vol;
    Sense sense;
  };

  // Private methods

  /// Get a modifyable reference to the underlying libmesh mesh.
  MeshBase& mesh();

  /// Get a modifyable reference to the underlying libmesh equation systems
  EquationSystems & systems();

  /// Get a modifyable reference to the underlying libmesh system
  System& system(std::string var_now);

  // Helper methods to set MOAB database

  /// Helper method to create MOAB nodes
  moab::ErrorCode createNodes(std::map<dof_id_type,moab::EntityHandle>& node_id_to_handle);
  /// Helper method to create MOAB elements
  void createElems(std::map<dof_id_type,moab::EntityHandle>& node_id_to_handle);

  /// Helper method to create MOAB tags
  moab::ErrorCode createTags();

  /// Helper method to create MOAB group entity set
  moab::ErrorCode createGroup(unsigned int id, std::string name,moab::EntityHandle& group_set);

  /// Helper method to create MOAB volume entity set
  moab::ErrorCode createVol(unsigned int id,moab::EntityHandle& volume_set,moab::EntityHandle group_set);

  /// Helper method to create MOAB surface entity set
  moab::ErrorCode createSurf(unsigned int id,moab::EntityHandle& surface_set, moab::Range& faces,  std::vector<VolData> & voldata);

  /// Helper method to create MOAB surfaces with no overlaps
  moab::ErrorCode createSurfaces(moab::Range& reversed, VolData& voldata, unsigned int& surf_id);

  /// Create a MOAB surface from a bounding box
  moab::ErrorCode createSurfaceFromBox(const BoundingBox& box, VolData& voldata, unsigned int& surf_id, bool normalout, double factor=1.0);

  /// Create MOAB nodes from a bounding box
  moab::ErrorCode createNodesFromBox(const BoundingBox& box,double factor,std::vector<moab::EntityHandle>& vert_handles);

  /// Create 3 tri faces stemming from one corner of a cude (an open tetrahedron)
  moab::ErrorCode createCornerTris(const std::vector<moab::EntityHandle> & verts,
                                   unsigned int corner, unsigned int v1,
                                   unsigned int v2 ,unsigned int v3,
                                   bool normalout, moab::Range &surface_tris);

  /// Create MOAB tri surface element
  moab::ErrorCode createTri(const std::vector<moab::EntityHandle> & vertices,unsigned int v1, unsigned int v2 ,unsigned int v3, moab::Range &surface_tris);

  /// Add parent-child metadata relating a surface to its volume
  moab::ErrorCode updateSurfData(moab::EntityHandle surface_set,VolData data);

  /// Generic method to set the tags that DAGMC requires
  moab::ErrorCode setTags(moab::EntityHandle ent,std::string name, std::string category, unsigned int id, int dim);

  /// Helper function to wrap moab::tag_set_data for a string
  moab::ErrorCode setTagData(moab::Tag tag, moab::EntityHandle ent, std::string data, unsigned int SIZE);

  /// Helper function to wrap moab::tag_set_data for a generic pointer
  moab::ErrorCode setTagData(moab::Tag tag, moab::EntityHandle ent, void* data);

  /// Return all sets of node indices for sub-tetrahedra if we have a second order mesh
  bool getTetSets(ElemType type, std::vector< std::vector<unsigned int> > &perms);

  /// Build the graveyard (needed by OpenMC)
  moab::ErrorCode buildGraveyard(unsigned int & vol_id, unsigned int & surf_id);

  /// Get the coords of the box back as an array (possibly scaled)
  std::vector<Point> boxCoords(const BoundingBox& box, double factor);

  /// Look for materials in the FE problem
  void findMaterials();

  /// Clear the maps between entity handles and dof ids
  void clearElemMaps();

  /// Add an element to maps
  void addElem(dof_id_type id,moab::EntityHandle ent);

  /// Helper method to set the results in a given system and variable
  void setSolution(unsigned int iSysNow, unsigned int iVarNow,std::vector< double > &results, double scaleFactor, bool isErr, bool normToVol);

  /// Helper method to convert between elem / solution indices
  dof_id_type elem_to_soln_index(const Elem& elem,unsigned int iSysNow, unsigned int iVarNow);

  /// Get a serialised version of solution for a given system
  NumericVector<Number>& getSerialisedSolution(libMesh::System* sysPtr);

  /// Create and save a mesh function for the provided variable
  void setMeshFunction(std::string var_name_in);

  /// Evaluate a mesh function at a point
  double evalMeshFunction(std::shared_ptr<MeshFunction> meshFunctionPtr,
                          const Point& p);

  /// Fetch the mesh function associated with a variable
  std::shared_ptr<MeshFunction> getMeshFunction(std::string var_name_in);

  /// Sort elems in to bins of a given temperature
  bool sortElemsByResults();

  /// Group the binned elems into local temperature regions and find their surfaces
  bool findSurfaces();

  /// Group a given bin into local regions
  /// NB elems in param is a copy, localElems is a reference
  void groupLocalElems(std::set<dof_id_type> elems, std::vector<moab::Range>& localElems);

  /// Given a value of our variable, find what bin this corresponds to.
  int getResultsBin(double value);
  /// Find results bin if we have linear binning
  inline int getResultsBinLin(double value);
    /// Find results bin if we have logarithmic
  int getResultsBinLog(double value);
  /// Return the bin index of a given relative density
  inline int getRelDensityBin(double value);

  /// Map material, density and temp bin indices onto a linearised index
  int getSortBin(int iVarBin, int iDenBin, int iMat,
                 int nVarBinsIn, int nDenBinsIn,int nMatsIn);
  /// Map material, density and temp bin indices onto a linearised index
  /// with default parameters for number of bins
  int getSortBin(int iVarBin, int iDenBin, int iMat){
    return getSortBin(iVarBin,iDenBin,iMat,nVarBins,nDenBins,nMatBins);
  }

  /// Map density and temp bin indices onto a linearised index
  int getMatBin(int iVarBin, int iDenBin, int nVarBinsIn, int nDenBinsIn);
  /// Map density and temp bin indices onto a linearised index
  /// with default parameters for number of bins
  int getMatBin(int iVarBin, int iDenBin){
    return getMatBin(iVarBin,iDenBin,nVarBins,nDenBins);
  }

  /// Calculate the variable evaluated at the bin midpoints
  void calcMidpoints();
  /// Calculate the variable evaluated at the bin midpoints for linear binning
  void calcMidpointsLin();
  /// Calculate the variable evaluated at the bin midpoints for log binning
  void calcMidpointsLog();
  /// Calculate the density evaluated at the bin midpoints
  void calcDenMidpoints();

  /// Calculate a generic variable midpoints given binning params
  void calcMidpointsLin(double var_min_in, double bin_width_in,int nbins_in,std::vector<double>& midpoints_in);

  /// Return the centroid position of an element
  Point elemCentroid(Elem& elem);

  /// Clear the containers of elements grouped into bins of constant temp
  void resetContainers();

  /// Clear MOAB entity sets
  bool resetMOAB();

  /// Find the surfaces for the provided range and add to group
  bool findSurface(const moab::Range& region,moab::EntityHandle group, unsigned int & vol_id, unsigned int & surf_id,moab::EntityHandle& volume_set);

  /// Write to file
  bool write();

  /// MPI communication of DOFs of binned elements
  void communicateDofSet(std::set<dof_id_type>& dofset);

  /// Pointer to the feProblem we care about
  FEProblemBase * _problem_ptr;

  /// Pointer to a moab skinner for finding temperature surfaces
  std::unique_ptr< moab::Skinner > skinner;

  /// Pointer for gtt for setting surface sense
  std::unique_ptr< moab::GeomTopoTool > gtt;

  /// Convert MOOSE units to dagmc length units
  double lengthscale;

  /// Convert MOOSE density units to openmc density units
  double densityscale;

  /// Map from libmesh id to MOAB element entity handles
  std::map<dof_id_type,std::vector<moab::EntityHandle> > _id_to_elem_handles;

  /// Save the first tet entity handle
  moab::EntityHandle offset;

  // Data members relating to binning in temperature

  /// Name of the MOOSE variable
  std::string var_name;
  /// Whether or not to perform binning
  bool binElems;
  /// Whether or not to bin in a log scale
  bool logscale;
  /// Minimum value of our variable
  double var_min;
  /// Maximum value of our variable for binning on a linear scale
  double var_max;
  /// Fixed bin width for binning on a linear scale
  double bin_width;
  /// Minimum power of 10
  int powMin;
  /// Maximum power of 10
  int powMax;
  /// Number of variable bins to use
  unsigned int nVarBins;
  /// Number of powers of 10 to bin in for binning on a log scale
  unsigned int nPow;
  /// Number of minor divisions for binning on a log scale
  unsigned int nMinor;
  /// Store the temperature corresponding to the bin mipoint
  std::vector<double> midpoints;
  /// Store the relative density corresponding to the bin mipoint
  std::vector<double> den_midpoints;

  /// Data members relating to  binning in density

  /// Name of the MOOSE variable containing the density
  std::string den_var_name;
  /// Switch to determine if we should bin by material density
  bool binByDensity;
  /// Minimum relative density diff
  double rel_den_min;
  /// Max relative density diff
  double rel_den_max;
  /// Relative density diff bin width
  double rel_den_bw;
  /// Number of relative density bins
  unsigned int nDenBins;
  /// Number of distinct subdomains (e.g. vols, mats)
  unsigned int nMatBins;

  /// Container for elems sorted by variable bin and materials
  std::vector<std::set<dof_id_type> > sortedElems;

  /// A map to store mesh functions against their variable name
  std::map<std::string, std::shared_ptr<MeshFunction> > meshFunctionPtrs;

  /// A place to store the entire solution
  // N.B. For big problems this is going to be a memory bottleneck
  // TODO: We will need to come up with a better solution
  // Map is from systems index
  std::map<unsigned int, std::unique_ptr<NumericVector<Number> > > serial_solutions;

  // Materials data

  /// material names
  std::vector<std::string> mat_names;
  /// OpenMC material names
  std::vector<std::string> openmc_mat_names;
  /// all element blocks assigned to mats
  std::vector< std::set<SubdomainID> > mat_blocks;
  /// vector for initial densities if binning by density
  std::vector<double> initialDensities;

  /// An entitiy handle to represent the set of all tets
  moab::EntityHandle meshset;

  /// Save some topological data: map from surface handle to vol handle and sense
  std::map<moab::EntityHandle, std::vector<VolData> > surfsToVols;

  // Some moab tags

  /// Tag for dimension for geometry
  moab::Tag geometry_dimension_tag;
  /// Tag for entitiy set ID
  moab::Tag id_tag;
  /// Tag for faceting tolerance
  moab::Tag faceting_tol_tag;
  /// Tag needed by DAGMC
  moab::Tag geometry_resabs_tag;
  /// Tag for type of entity set
  moab::Tag category_tag;
  /// Tag for name of entity set
  moab::Tag name_tag;

  /// Const to encode that MOAB tets have 4 nodes
  const unsigned int nNodesPerTet = 4;

  // DagMC settings
  /// Faceting tolerence needed by DAGMC
  double faceting_tol;
  /// Geometry tolerence needed by DAGMC
  double geom_tol;

  /// Scalefactors applied to bounding box for inner surface of graveyard
  double scalefactor_inner;
  /// Scalefactors applied to bounding box for outer surface of graveyard
  double scalefactor_outer;

  // Settings to control the optional writing of surfaces to file.

  /// Flag to control whether to save surface skins to file
  bool output_skins;
  /// Flag to control whether to output the full MOAB mesh database
  bool output_full;
  /// Base name of skins output file
  std::string output_base;
  /// Base name of full database output file
  std::string output_base_full;
  /// Number of times to write to file
  unsigned int n_output;
  /// Period of writes (skip every n_period -1)
  unsigned int n_period;
  /// Count number of times file has been written to
  unsigned int n_write;
  /// Store the number of times writeSurfaces is called
  unsigned int n_its;

  /// Performance timer for initialisation
  PerfID _init_timer;
  /// Performance timer for update
  PerfID _update_timer;
  /// Performance timer for setSolution
  PerfID _setsolution_timer;

};
