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

#pragma once

#include "MooseMesh.h"
#include "MooseApp.h"
#include "CardinalEnums.h"
#include "NekBoundaryCoupling.h"
#include "NekVolumeCoupling.h"
#include "NekInterface.h"
#include "NekUtility.h"

/**
 * Representation of a nekRS surface mesh as a native MooseMesh. This is
 * constructed by interpolating from the surface Gauss-Lobatto-Legendre points
 * in nekRS to either a first-order (Quad4) or second-order (Quad9) mesh. This
 * mesh is only constructed for a user-specified set of boundaries in the nekRS
 * mesh with the 'boundary' parameter. Therefore, this class contains a mixture of
 * information related to the nekRS mesh (that nekRS solves its equations on)
 * versus the surface mesh constructed for data transfer with MOOSE (which is
 * only used by nekRS for the purpose of transferring its solution). All information
 * specific to the mesh nekRS actually uses for its solution are prefaced with either
 * '_nek' or 'nek' to help with this distinction.
 *
 * The nekRS mesh is currently implemented as a replicated mesh. On the nekRS side,
 * an Allgather is used to get the surface geometry information on each
 * nekRS process such that access from MOOSE can be performed on each process.
 *
 * TODO: The extension to higher than a second-order representation requires
 * some modifications to the formation of the mesh, as well as the interpolation
 * matrices used in NekRSProblem, because for 3rd order of higher, the equispaced
 * libMesh nodes no longer are a subset of the GLL ndoes.
 */
class NekRSMesh : public MooseMesh
{
public:
  NekRSMesh(const InputParameters & parameters);
  static InputParameters validParams();

  NekRSMesh(const NekRSMesh & /* other_mesh */) = default;

  NekRSMesh & operator=(const NekRSMesh & other_mesh) = delete;
  virtual std::unique_ptr<MooseMesh> safeClone() const override;

  /// Save the initial volumetric mesh for volume mirror-based moving mesh problems
  void saveInitialVolMesh();

  /// Initialize previous displacement values to zero for boundary mirror-based moving mesh problems
  void initializePreviousDisplacements();

  /**
   * NekRS mesh polynomial order
   * @return NekRS polynomial order
   */
  int nekPolynomialOrder() const { return _nek_polynomial_order; }

  /**
   * Number of elements to build for each NekRS volume element
   * @return number of MOOSE elements per NekRS volume element
   */
  int nBuildPerVolumeElem() const { return _n_build_per_volume_elem; }

  /**
   * Number of elements to build for each NekRS surface element
   * @return number of MOOSE elements per NekRS surface element
   */
  int nBuildPerSurfaceElem() const { return _n_build_per_surface_elem; }

  /**
   * Get the initial mesh x coordinates
   * @return initial mesh x coordinates
   */
  const std::vector<double> & nek_initial_x() const { return _initial_x; }

  /**
   * Get the initial mesh y coordinates
   * @return initial mesh y coordinates
   */
  const std::vector<double> & nek_initial_y() const { return _initial_y; }

  /**
   * Get the initial mesh z coordinates
   * @return initial mesh z coordinates
   */
  const std::vector<double> & nek_initial_z() const { return _initial_z; }

  /**
   * Get the previous x displacement
   * @return previous x displacement values
   */
  std::vector<double> & prev_disp_x() { return _prev_disp_x; }

  /**
   * Get the previous y displacement
   * @return previous y displacement values
   */
  std::vector<double> & prev_disp_y() { return _prev_disp_y; }

  /**
   * Get the previous z displacement
   * @return previous z displacement values
   */
  std::vector<double> & prev_disp_z() { return _prev_disp_z; }

  /**
   * Get the boundary coupling data structure
   * @return boundary coupling data structure
   */
  const NekBoundaryCoupling & boundaryCoupling() const { return _boundary_coupling; }

  /**
   * Get the volume coupling data structure
   * @return volume coupling data structure
   */
  const NekVolumeCoupling & volumeCoupling() const { return _volume_coupling; }

  /// Add all the elements in the mesh to the MOOSE data structures
  virtual void addElems();

  /**
   * Get the order of the surface mesh; note that this is zero-indexed, so 0 = first, 1 = second
   * \return order
   */
  const order::NekOrderEnum & order() const { return _order; }

  /**
   * Get the number of quadrature points per coordinate direction in MOOSE's representation of
   * nekRS's mesh \return number of quadrature points per coordinate direction
   */
  int numQuadraturePoints1D() const;

  /**
   * Get the number of quadrature points per coordiante direction in nekRS's mesh
   * \return number of quadrature points per coordinate direction
   */
  int nekNumQuadraturePoints1D() const;

  /**
   * \brief Get the number of NekRS elements we rebuild in the MOOSE mesh
   *
   * This function is used to perform the data transfer routines in NekRSProblem
   * agnostic of whether we have surface or volume coupling.
   * return number of elements
   */
  const int & numElems() const { return _n_elems; }

  /**
   * \brief Get the number of vertices per element in MOOSE's representation of nekRS's mesh
   *
   * This function is used to perform the data transfer routines in NekRSProblem
   * agnostic of whether we have surface or volume coupling.
   * return number of vertices per element
   */
  const int & numVerticesPerElem() const { return _n_vertices_per_elem; }

  /**
   * \brief Get the libMesh node index from nekRS's GLL index ordering
   *
   * This function is used to perform the data transfer routines in NekRSProblem
   * agnostic of whether we have surface or volume coupling.
   * @param[in] gll_index nekRS GLL index
   * @return node index
   */
  int nodeIndex(const int gll_index) const { return (*_node_index)[gll_index]; }

  /**
   * Get the number of surface elements in MOOSE's representation of nekRS's mesh
   * \return number of surface elements
   */
  const int & numSurfaceElems() const { return _n_surface_elems; }

  /**
   * Get the total number of surface elements in nekRS's mesh
   * \return number of surface elements
   */
  const int & nekNumSurfaceElems() const { return _nek_n_surface_elems; }

  /**
   * Get the number of vertices per surface element in MOOSE's representation of nekRS's mesh
   * \return number of vertices per surface
   */
  const int & numVerticesPerSurface() const { return _n_vertices_per_surface; }

  /**
   * Get the number of volume elements in MOOSE's representation of nekRS's mesh
   * \return number of volume elements
   */
  const int & numVolumeElems() const { return _n_volume_elems; }

  /**
   * Get the total number of volume elements in nekRS's mesh
   * \return number of volume elements
   */
  const int & nekNumVolumeElems() const { return _nek_n_volume_elems; }

  /**
   * Get the number of vertices per volume element in MOOSE's representation of nekRS's mesh
   * \return number of vertices per volume
   */
  const int & numVerticesPerVolume() const { return _n_vertices_per_volume; }

  /**
   * Get the boundary ID for which nekRS and MOOSE are coupled
   * \return boundary ID
   */
  const std::vector<int> * boundary() const { return _boundary; }

  /**
   * Get whether the mesh permits volume-based coupling
   * \return whether mesh contains volume elements
   */
  const bool & volume() const { return _volume; }

  /// Create a new element for a boundary mesh
  Elem * boundaryElem() const;

  /// Create a new element for a volume mesh
  Elem * volumeElem() const;

  virtual void buildMesh() override;

  /**
   * If running NekRS in JIT mode, we still need to make a mesh based on requirements
   * in MOOSE, so we just make a dummy mesh of a single Quad4 element
   */
  virtual void buildDummyMesh();

  /**
   * For the case of surface coupling only (i.e. no volume coupling), we create a surface
   * mesh for the elements on the specified boundary IDs
   */
  virtual void extractSurfaceMesh();

  /**
   * For the case of volume coupling only (i.e. no surface coupling), we create a volume
   * mesh for all volume elements
   */
  virtual void extractVolumeMesh();

  /**
   * Get the libMesh node index from nekRS's GLL index ordering
   * @param[in] gll_index nekRS GLL index
   * @return node index
   */
  int boundaryNodeIndex(const int gll_index) const { return _bnd_node_index[gll_index]; }

  /**
   * Get the libMesh node index from nekRS's GLL index ordering
   * @param[in] gll_index nekRS GLL index
   * @return node index
   */
  int volumeNodeIndex(const int gll_index) const { return _vol_node_index[gll_index]; }

  /**
   * Get the scaling factor applied to the nekRS mesh
   * @return scaling factor
   */
  const Real & scaling() const { return _scaling; }

  /// Print diagnostic information related to the mesh
  virtual void printMeshInfo() const;

  /**
   * Processor id (rank) owning the given boundary element
   * @return processor id
   */
  int boundaryElemProcessorID(const int elem_id);

  /**
   * Processor id (rank) owning the given volume element
   * @return processor id
   */
  int volumeElemProcessorID(const int elem_id);

  /**
   * Get the number of faces of this global element that are on a coupling boundary
   * @param[in] elem_id global element ID
   * @return number of faces on a coupling boundary
   */
  int facesOnBoundary(const int elem_id) const;

  /**
   * Whether the mesh mirror is an exact representation of the NekRS mesh
   * @param return whether mesh mirror is exact
   */
  bool exactMirror() const { return _exact; }

  /**
   * Get the corner indices for the GLL points to be used in the mesh mirror
   * @return mapping of mesh mirror nodes to GLL points
   */
  std::vector<std::vector<int>> cornerIndices() const { return _corner_indices; }

  /**
   * Get the number of MOOSE elements we build for each NekRS element
   * @return MOOSE elements built for each NekRS element
   */
  int nMoosePerNek() const { return _n_moose_per_nek; }

  /**
   * Copy a new boundary displacement value for a given element's face
   * @param[in] src the source displacement at element e and face f
   * @param[in] e the element to which src belongs
   * @param[in] field the displacement field we are updating
   */
  void updateDisplacement(const int e, const double *src, const field::NekWriteEnum field);

protected:
  /// Store the rank-local element and rank ownership for volume coupling
  void storeVolumeCoupling();

  /**
   * Store the rank-local element and rank ownership for boundary coupling;
   * this loops over the NekRS mesh and fetches relevant information on the boundaries
   */
  void storeBoundaryCoupling();

  /**
   * Sideset ID corresponding to a given volume element with give local face ID
   * @param[in] elem_id element local rank ID
   * @param[in] face_id element-local face ID
   * @return sideset ID (-1 means not one a boundary)
   */
  int boundary_id(const int elem_id, const int face_id);

  /**
   * Get the vertices defining the surface mesh interpolation from the
   * stored coupling information and store in _x, _y, and _z
   */
  void faceVertices();

  /**
   * Get the vertices defining the volume mesh interpolation from the
   * stored coupling information and store in _x, _y, and _z
   */
  void volumeVertices();

  /// Initialize members for the mesh and determine the GLL-to-node mapping
  void initializeMeshParams();

  /**
   * \brief Whether nekRS is coupled through volumes to MOOSE
   *
   * Unlike the case with _boundary, nekRS has no concept of volume/block IDs,
   * so we cannot have the user provide a vector of volumes that they want to
   * construct, so the best we can do is use a boolean here to turn on/off the
   * volume-based coupling for the entire mesh.
   */
  const bool & _volume;

  /// Boundary ID(s) through which to couple Nek to MOOSE
  const std::vector<int> * _boundary;

  /**
   * \brief Order of the surface interpolation between nekRS and MOOSE
   *
   * Order of the interpolation to be performed between nekRS and MOOSE;
   * options = FIRST, SECOND. For a first-order interpolation, nekRS's
   * solution is interpolated onto a first-order surface mesh (i.e. Quad4),
   * while for a second-order interpolation, nekRS's solution is interpolated
   * onto a second-order surface mesh (i.e. Quad9). Note that this is zero-indexed
   * so that an integer value of 0 = first-order, 1 = second-order, etc.
   **/
  const order::NekOrderEnum _order;

  /**
   * Whether the NekRS mesh mirror is an exact replica of the NekRS mesh.
   * If false (the default), then we build one MOOSE element for each NekRS element,
   * and the order of the MOOSE element is selected with 'order'. If true,
   * then we instead build one MOOSE element for each "first-order element"
   * within each NekRS high-order spectral element. In other words, if the NekRS
   * mesh is polynomial order 7, then we would build 7^2 MOOSE surface elements
   * for each NekRS surface element, and 7^3 MOOSE volume elements for each NekRS
   * volume element. The order of these elements will be first order.
   */
  const bool & _exact;

  /// Number of vertices per surface
  int _n_vertices_per_surface;

  /// Number of vertices per volume element
  int _n_vertices_per_volume;

  /**
   * \brief Spatial scaling factor to apply to the mesh
   *
   * nekRS is dimension agnostic - depending on the values used for the material
   * properties, the units of the mesh are arbitrary. Other apps that nekRS might
   * be coupled to could be in different units - to allow each app to use the
   * units that it wants, we can simply scale the NekRSMesh by a constant factor.
   * This will also adjust the heat flux coming in to nekRS by an appropriate factor.
   * For instance, if nekRS solves a problem in units of meters, but a BISON solution
   * is done on a mesh in units of centimeters, this scaling factor should be set to
   * 100. Note that other postprocessors will still be calculated on the nekRS mesh,
   * which will be in whatever units nekRS is internally using.
   */
  const Real & _scaling;

  /// Block ID for the fluid portion of the mesh mirror
  const unsigned int & _fluid_block_id;

  /// Block ID for the solid portion of the mesh mirror
  const unsigned int & _solid_block_id;

  /// Order of the nekRS solution
  int _nek_polynomial_order;

  /**
   * Number of NekRS surface elements in MooseMesh. The total number of surface
   * elements in the mesh mirror is _n_surface_elems * _n_build_per_surface_elem.
   */
  int _n_surface_elems;

  /// Number of MOOSE surface elements to build per NekRS surface element
  int _n_build_per_surface_elem;

  /**
   * Number of NekRS volume elements in MooseMesh. The total number of volume
   * elements in the mesh mirror is _n_volume_elems * _n_build_per_volume_elem.
   */
  int _n_volume_elems;

  /// Number of MOOSE volume elements to build per NekRS volume element
  int _n_build_per_volume_elem;

  /**
   * Number of NekRS elements we build in the MooseMesh. The total number of
   * elements in the mesh mirror is _n_elems * _n_moose_per_nek.
   */
  int _n_elems;

  /**
   * Number of MOOSE elements corresponding to each NekRS element, which depends on whether
   * building a boundary/volume mesh
   */
  int _n_moose_per_nek;

  /// Function returning the processor id which should own each element
  int (NekRSMesh::*_elem_processor_id)(const int elem_id);

  /// Number of vertices per element, which depends on whether building a boundary/volume mesh
  int _n_vertices_per_elem;

  /// Mapping of GLL nodes to libMesh nodes, which depends on whether building a boundary/volume mesh
  std::vector<int> * _node_index;

  /// Total number of surface elements in the nekRS problem
  int _nek_n_surface_elems;

  /// Total number of volume elements in the nekRS problem
  int _nek_n_volume_elems;

  /**
   * \brief "Phase" for each element (fluid = 0, solid = 1)
   *
   * TODO: This could be improved so that we don't need to collect this information,
   * but would require rewriting of a lot of the code used to assemble the members in
   * NekVolumeCoupling.
   */
  std::vector<int> _phase;

  /**
   * \brief \f$x\f$ coordinates of the current GLL points (which can move in time), for this rank
   *
   * This is ordered according to nekRS's internal geometry layout, and is indexed
   * first by the element and then by the node.
   */
  std::vector<double> _x;

  /**
   * \brief \f$y\f$ coordinates of the current GLL points (which can move in time), for this rank
   *
   * This is ordered according to nekRS's internal geometry layout, and is indexed
   * first by the element and then by the node.
   */
  std::vector<double> _y;

  /**
   * \brief \f$z\f$ coordinates of the current GLL points (which can move in time), for this rank
   *
   * This is ordered according to nekRS's internal geometry layout, and is indexed
   * first by the element and then by the node.
   */
  std::vector<double> _z;

  /**
   * \brief \f$x\f$ coordinates of the initial GLL points, for this rank
   *
   * This is ordered according to nekRS's internal geometry layout, and is indexed
   * first by the element and then by the node.
   */
  std::vector<double> _initial_x;

  /**
   * \brief \f$y\f$ coordinates of the initial GLL points, for this rank
   *
   * This is ordered according to nekRS's internal geometry layout, and is indexed
   * first by the element and then by the node.
   */
  std::vector<double> _initial_y;

  /**
   * \brief \f$z\f$ coordinates of the initial GLL points, for this rank
   *
   * This is ordered according to nekRS's internal geometry layout, and is indexed
   * first by the element and then by the node.
   */
  std::vector<double> _initial_z;

  ///@{
  /**
   * \f$x\f$, \f$y\f$, \f$z\f$ displacements of the boundary mesh mirror
   * for calculating displacement, in this rank
   **/
  std::vector<double> _prev_disp_x;
  std::vector<double> _prev_disp_y;
  std::vector<double> _prev_disp_z;
  ///@}
  /**
   * \brief Mapping of boundary GLL indices to MooseMesh node indices
   *
   * In nekRS, the GLL points are ordered by \f$x\f$, \f$y\f$, and \f$z\f$ coordinates,
   * but in order to construct sensible elements in Moose, we need to reorder these
   * points so that they match a libMesh-friendly node ordering. Without such a mapping,
   * we would construct triangles with zero/negative Jacobians instead of quad elements.
   * By indexing in the GLL index, this returns the node index.
   **/
  std::vector<int> _bnd_node_index;

  /**
   * \brief Mapping of volume GLL indices to MooseMesh node indices
   *
   * In nekRS, the GLL points are ordered by \f$x\f$, \f$y\f$, and \f$z\f$ coordinates,
   * but in order to construct sensible elements in Moose, we need to reorder these
   * points so that they match a libMesh-friendly node ordering. Without such a mapping,
   * we would construct triangles with zero/negative Jacobians instead of hex elements.
   * By indexing in the GLL index, this returns the node index.
   **/
  std::vector<int> _vol_node_index;

  /**
   * \brief Mapping of side indices to libMesh side indices
   *
   * nekRS uses its own side mapping that differs from that assumed in libMesh. In order
   * to assign the correct sideset IDs to the MooseMesh, we need to know the mapping between
   * these different conventions. By indexing in the nekRS side index, this returns the
   * libMesh side index.
   */
  std::vector<int> _side_index;

  /// Function pointer to the type of new element to add
  Elem * (NekRSMesh::*_new_elem)() const;

  /// Data structure holding mapping information for boundary coupling
  NekBoundaryCoupling _boundary_coupling;

  /// Data structure holding mapping information for volume coupling
  NekVolumeCoupling _volume_coupling;

  /// Pointer to NekRS's internal mesh data structure
  mesh_t * _nek_internal_mesh = nullptr;

  /// Corner indices for GLL points of mesh mirror elements
  std::vector<std::vector<int>> _corner_indices;
};
