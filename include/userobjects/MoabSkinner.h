#pragma once

#include "GeneralUserObject.h"
#include "MaterialBase.h"
#include "MooseMesh.h"

#include "moab/Core.hpp"
#include "moab/Skinner.hpp"
#include "moab/GeomTopoTool.hpp"
#include "MBTagConventions.hpp"

/**
 * \brief Skins the [Mesh] according to individual bins for temperature, density, and subdomain ID
 *
 * Skins a [Mesh] according to temperature, density, and subdomain. The MOAB surfaces bounding
 * those grouped elements are then generated, providing geometry information needed for DAGMC
 * to then track particles on this new geometry.
 */
class MoabSkinner : public GeneralUserObject
{
public:
  MoabSkinner(const InputParameters & parameters);

  static InputParameters validParams();

  virtual void execute() override;

  virtual void initialize() override;

  virtual void finalize() override;

  virtual void threadJoin(const UserObject & /* uo */) override {}

  /**
   * Wrap the error handling in MOAB to print errors to user
   * @param[in] input MOAB error code
   * @return error mode
   */
  virtual moab::ErrorCode check(const moab::ErrorCode input) const;

  std::string materialName(const unsigned int & block,
                           const unsigned int & density,
                           const unsigned int & temp) const;

  /// Perform the skinning operation
  virtual void update();

  /**
   * Set the names to be used for naming the subdomains in the skinned mesh;
   * there should be one name per subdomain.
   * @param[in] names names for subdomains
   */
  virtual void setMaterialNames(std::vector<std::string> names) { _material_names = names; }

  /**
   * Get the total number of bins
   * @return total number of bins
   */
  unsigned int nBins() const;

  /**
   * Get the bin index for the temperature
   * @param[in] elem element
   * @return temperature bin index
   */
  virtual unsigned int getTemperatureBin(const Elem * const elem) const;

  /**
   * Get the bin index for the density
   * @param[in] elem element
   * @return density bin index
   */
  virtual unsigned int getDensityBin(const Elem * const elem) const;

  /**
   * Get the bin index for the subdomain
   * @param[in] elem element
   * @return subdomain bin index
   */
  virtual unsigned int getSubdomainBin(const Elem * const elem) const
  {
    return _blocks.at(elem->subdomain_id());
  }

  /**
   * Override the user parameter for use_displaced
   * @param[in] use whether to use the displaced mesh
   */
  void setUseDisplacedMesh(const bool & use);

  /**
   * Set the length multiplier to get from [Mesh] units into centimeters
   * @param[in] scale multiplier
   */
  virtual void setScaling(const Real & scale) { _scaling = scale; }

  /**
   * Set the verbosity level
   * @param[in] verbose whether to print diagnostic information
   */
  virtual void setVerbosity(const bool & verbose) { _verbose = verbose; }

  /**
   * Indicate whether this userobject is run by itself (for testing purposes)
   * or controlled by some other class.
   */
  virtual void makeDependentOnExternalAction() { _standalone = false; }

  /**
   * Get variable number in the auxiliary system
   * @param[in] name variable name
   * @param[in] param_name parameter name, for printing a helpful error message
   * @return variable number
   */
  unsigned int getAuxiliaryVariableNumber(const std::string & name,
                                          const std::string & param_name) const;

  /// Clear mesh data
  void reset();

  /**
   * Get total bin index given individual indices for the temperature, density, and subdomain bins
   * @param[in] temp_bin temperature bin
   * @param[in] density_bin density bin
   * @param[in] subdomain_bin subdomain ID bin
   * @return total bin index
   */
  virtual unsigned int getBin(const unsigned int & temp_bin,
                              const unsigned int & density_bin,
                              const unsigned int & subdomain_bin) const;

  /**
   * Whether the skinner builds a graveyard
   * @return whether a graveyard is built
   */
  virtual const bool & hasGraveyard() const { return _build_graveyard; }

  /**
   * Set the graveyard setting
   * @param[in] build whether to build a graveyard
   */
  void setGraveyard(bool build);

  /**
   * Number of density bins; if greater than 1, this means we must be re-generating
   * OpenMC materials during the course of the simulation.
   * @return number of density bins
   */
  virtual unsigned int nDensityBins() const { return _n_density_bins; }

  /**
   * Whether density skinning is applied
   * @return using density skinning
   */
  virtual bool hasDensitySkinning() const { return _bin_by_density; }

  /**
   * Get pointer to underlying moab interface
   * @return pointer to moab interface
   */
  const std::shared_ptr<moab::Interface> & moabPtr() const { return _moab; }

protected:
  std::unique_ptr<NumericVector<Number>> _serialized_solution;

  /// MOAB interface
  std::shared_ptr<moab::Interface> _moab;

  /// Whether to print diagnostic information
  bool _verbose;

  /// Name of the temperature variable
  const std::string & _temperature_name;

  /// Lower bound of temperature bins
  const Real & _temperature_min;

  /// Upper bound of temperature bins
  const Real & _temperature_max;

  /// Number of temperature bins
  const unsigned int & _n_temperature_bins;

  /// Temperature bin width
  const Real _temperature_bin_width;

  /// Whether elements are binned by density (in addition to temperature and block)
  const bool _bin_by_density;

  /// Material names corresponding to each subdomain. These are used to name the
  /// new skinned volumes in MOAB
  std::vector<std::string> _material_names;

  /// Faceting tolerence needed by DAGMC
  const Real & _faceting_tol;

  /// Geometry tolerence needed by DAGMC
  const Real & _geom_tol;

  /// Multiplier on bounding box for inner surface of graveyard
  const Real & _graveyard_scale_inner;

  /// Multiplier on bounding box for outer surface of graveyard
  const Real & _graveyard_scale_outer;

  /// Whether to output the MOAB mesh skins to a .h5m file
  const bool & _output_skins;

  /// Whether to output the MOAB mesh to a .h5m file
  const bool & _output_full;

  /**
   * Whether to build a graveyard as two additional cube surfaces surrounding the mesh.
   * This is only needed if the skinned geometry is fed into a Monte Carlo code.
   */
  bool _build_graveyard;

  /// Whether to assign a material to the implicit complement region
  bool _set_implicit_complement_material = false;

  /// OpenMC material name or ID which will be assigned to the implicit complement
  std::string _implicit_complement_group_name;

  /// Whether the skinned mesh should be generated from a displaced mesh
  bool _use_displaced;

  /// Length multiplier to get from [Mesh] units into OpenMC's centimeters
  Real _scaling;

  /// Count number of times output files have been written
  unsigned int _n_write;

  /// Whether this class runs by itself, or is controlled by an external class
  bool _standalone;

  /// Encode the whether the surface normal faces into or out of the volume
  enum Sense
  {
    BACKWARDS = -1,
    FORWARDS = 1
  };

  /// Encode MOAB information about volumes needed when creating surfaces
  struct VolData
  {
    moab::EntityHandle vol;
    Sense sense;
  };

  /// Moose mesh
  MooseMesh & getMooseMesh();
  /**
   * Copy the libMesh [Mesh] into a MOAB mesh. This first loops through all of the
   * nodes, and rebuilds each as a MOAB vertex. Then, we loop over all of the elements
   * and rebuild each as a TET4 (if the libMesh mesh has TET10 elements, they are each
   * rebuilt into 8 TET4 elements).
   */
  void createMOABElems();

  /// Helper method to create MOAB tags
  virtual void createTags();

  /**
   * Helper method to create MOAB group entity set
   * @param[in] id ID for the group
   * @param[in] name name for the group
   * @param[in] group_set group of entities
   */
  void
  createGroup(const unsigned int & id, const std::string & name, moab::EntityHandle & group_set);

  /// Helper method to create MOAB volume entity set
  void
  createVol(const unsigned int & id, moab::EntityHandle & volume_set, moab::EntityHandle group_set);

  /// Helper method to create MOAB surface entity set
  void createSurf(const unsigned int & id,
                  moab::EntityHandle & surface_set,
                  moab::Range & faces,
                  const std::vector<VolData> & voldata);

  /// Helper method to create MOAB surfaces with no overlaps
  void createSurfaces(moab::Range & reversed, VolData & voldata, unsigned int & surf_id);

  /**
   * Create a MOAB surface from a bounding box
   */
  void createSurfaceFromBox(const BoundingBox & box,
                            const VolData & voldata,
                            unsigned int & surf_id,
                            bool normalout,
                            const Real & factor);

  /**
   * Create MOAB nodes from a bounding box
   * @param[in] box bounding box
   * @param[in] factor multiplicative factor to resize the bounding box sides
   * @return nodes
   */
  std::vector<moab::EntityHandle> createNodesFromBox(const BoundingBox & box,
                                                     const Real & factor) const;

  /// Create 3 tri faces stemming from one corner of a cude (an open tetrahedron)
  void createCornerTris(const std::vector<moab::EntityHandle> & verts,
                        unsigned int corner,
                        unsigned int v1,
                        unsigned int v2,
                        unsigned int v3,
                        bool normalout,
                        moab::Range & surface_tris);

  /// Create MOAB tri surface element
  moab::EntityHandle createTri(const std::vector<moab::EntityHandle> & vertices,
                               unsigned int v1,
                               unsigned int v2,
                               unsigned int v3);

  /// Add parent-child metadata relating a surface to its volume
  void updateSurfData(moab::EntityHandle surface_set, const VolData & data);

  /// Generic method to set the tags that DAGMC requires
  void
  setTags(moab::EntityHandle ent, std::string name, std::string category, unsigned int id, int dim);

  /// Helper function to wrap moab::tag_set_data for a string
  void setTagData(moab::Tag tag, moab::EntityHandle ent, std::string data, unsigned int SIZE);

  /// Helper function to wrap moab::tag_set_data for a generic pointer
  void setTagData(moab::Tag tag, moab::EntityHandle ent, void * data);

  /**
   * Get the node numberings for the MOAB TET4 elements to build for each [Mesh] element
   * @param[in] type element type
   */
  const std::vector<std::vector<unsigned int>> & getTetSets(ElemType type) const;

  /**
   * \brief Build a graveyard volume around the domain
   *
   * The graveyard is a containing volume which bounds the volume of interest. This is
   * only needed if the skinned geometry is going to be input into a Monte Carlo solver. For
   * performance reasons, a cubic shell is optimal. So, here we build two cubic surfaces,
   * both larger than the bounding box of the "actual" geometry. We name this region
   * "mat:Graveyard", so that when OpenMC parses the geometry it knows to assign "void"
   * to this region, and set vacuum BCs on the outer surfaces of the cubic shell. The
   * remaining space between the "actual" geometry and the inner graveyard surface is
   * treated as the implicit complement of the rest of the geometry (e.g. a transmissive region).
   */
  void buildGraveyard(unsigned int & vol_id, unsigned int & surf_id);

  /// Store a mapping from [Mesh] subdomain IDs to an index, to be used for binning by block ID
  virtual void findBlocks();

  /// Sort all the elements in the [Mesh] into bins for temperature, density, and subdomain.
  virtual void sortElemsByResults();

  /// Group the binned elems into local temperature regions and find their surfaces
  void findSurfaces();

  /// Add boundary condition groups to the DAGMC geometry
  void addBoundaryConditionGroups();

  /// Group a given bin into local regions
  /// NB elems in param is a copy, localElems is a reference
  void groupLocalElems(std::set<dof_id_type> elems, std::vector<moab::Range> & localElems);

  /// Clear MOAB entity sets
  bool resetMOAB();

  /// Find the surfaces for the provided range and add to group
  void findSurface(const moab::Range & region,
                   moab::EntityHandle group,
                   unsigned int & vol_id,
                   unsigned int & surf_id,
                   moab::EntityHandle & volume_set);

  /// Write MOAB volume and/or skin meshes to file
  virtual void write();

  /// Moab skinner for finding temperature surfaces
  std::unique_ptr<moab::Skinner> skinner;

  /// Topology tool for setting surface sense
  std::unique_ptr<moab::GeomTopoTool> gtt;

  /// Map from libmesh id to MOAB element entity handles
  std::map<dof_id_type, std::vector<moab::EntityHandle>> _id_to_elem_handles;

  /// Map from libMesh id to MOAB vertex handles
  std::map<dof_id_type, moab::EntityHandle> _node_id_to_handle;

  /// Save the first tet entity handle
  moab::EntityHandle offset;

  /// Name of the MOOSE variable containing the density
  std::string _density_name;

  /// Parsed list of vacuum boundary condition surfaces
  std::vector<BoundaryName> _vacuum_bcs_surfaces;

  /// Parsed list of reflective boundary condition surfaces
  std::vector<BoundaryName> _reflective_bcs_surfaces;

  /// Whether to assign boundary conditions to surfaces
  bool _set_bcs = false;

  /// Lower bound of density bins
  Real _density_min;

  /// Upper bound of density bins
  Real _density_max;

  /// Density bin width
  Real _density_bin_width;

  /// Number of density bins
  unsigned int _n_density_bins;

  /// Number of block bins
  unsigned int _n_block_bins;

  /// Mapping from total bin ID to a set of elements sorted into that bin
  std::vector<std::set<dof_id_type>> _elem_bins;

  /// Blocks in the [Mesh]
  std::map<SubdomainID, unsigned int> _blocks;

  /// Entity handle to represent the set of all tets
  moab::EntityHandle _all_tets;

  /// Save some topological data: map from surface handle to vol handle and sense
  std::map<moab::EntityHandle, std::vector<VolData>> surfsToVols;

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

  /// Bounds of the temperature bins
  std::vector<Real> _temperature_bin_bounds;

  /// Bounds of the density bins
  std::vector<Real> _density_bin_bounds;

  /// Node ordering for a TET4 MOAB element, based on libMesh node numberings
  std::vector<std::vector<unsigned int>> _tet4_nodes;

  /**
   * Node ordering for eight TET4 MOAB elements, based on libMesh node numberings
   * for a TET10 element. We re-build the libMesh element into first-order MOAB elements.
   */
  std::vector<std::vector<unsigned int>> _tet10_nodes;

  /// Auxiliary variable number for temperature
  unsigned int _temperature_var_num;

  /// Auxiliary variable number for density
  unsigned int _density_var_num;

  /// Number of nodes per MOAB tet (which are first order, so TET4)
  const unsigned int NODES_PER_MOAB_TET = 4;

  /// Tolerance to use for comparing values to bin bounds
  const Real BIN_TOLERANCE = 1e-6;
};
