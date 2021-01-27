//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseMesh.h"
#include "MooseApp.h"
#include "CardinalEnums.h"

class NekRSMesh;

template <>
InputParameters validParams<NekRSMesh>();

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
  NekRSMesh(const NekRSMesh & /* other_mesh */) = default;
  ~NekRSMesh();

  NekRSMesh & operator=(const NekRSMesh & other_mesh) = delete;

  virtual std::unique_ptr<MooseMesh> safeClone() const override;

  /**
   * Get the order of the surface mesh; note that this is zero-indexed, so 0 = first, 1 = second
   * \return order
   */
  const surface::NekOrderEnum & order() const;

  /**
   * Get the number of quadrature points per coordinate direction in MOOSE's representation of nekRS's mesh
   * \return number of quadrature points per coordinate direction
   */
  const int numQuadraturePoints1D() const;

  /**
   * Get the number of quadrature points per coordiante direction in nekRS's mesh
   * \return number of quadrature points per coordinate direction
   */
  const int nekNumQuadraturePoints1D() const;

  /**
   * Get the number of surface elements in MOOSE's representation of nekRS's mesh
   * \return number of surface elements
   */
  const int & numSurfaceElems() const;

  /**
   * Get the total number of surface elements in nekRS's mesh
   * \return number of surface elements
   */
  const int & nekNumSurfaceElems() const;

  /**
   * Get the number of vertices per face element in MOOSE's representation of nekRS's mesh
   * \return number of vertices per face
   */
  const int & numVerticesPerFace() const;

  /**
   * Get the boundary ID for which nekRS and MOOSE are coupled
   * \return boundary ID
   */
  const std::vector<int> & boundary() const;

  virtual void buildMesh() override;

  /**
   * Get the libMesh node index from nekRS's GLL index ordering
   * @param[in] gll_index nekRS GLL index
   * @return node index
   */
  int NodeIndex(const int gll_index) const { return _node_index[gll_index]; }

  /**
   * Get the nekRS GLL index from libMesh's node index ordering
   * @param[in] node_index libMesh node index
   * @return GLL index
   */
  int GLLIndex(const int node_index) const { return _gll_index[node_index]; }

  /**
   * Get the scaling factor applied to the nekRS mesh
   * @return scaling factor
   */
  const Real & scaling() const { return _scaling; }

protected:
  /// Initialize members for the mesh and determine the GLL-to-node mapping
  void initializeMeshParams();

  /// Boundary ID(s) through which to couple Nek to MOOSE
  const std::vector<int> _boundary;

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
  const surface::NekOrderEnum _order;

  /// Number of vertices per face
  int _n_vertices_per_face;

  /// Whether diagnostic information should be printed to the console
  const bool & _verbose;

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

  /// Order of the nekRS solution
  int _nek_polynomial_order;

  /// Number of surface elements in MooseMesh
  int _n_surface_elems;

  /// Total number of surface elements in the nekRS problem
  int _nek_n_surface_elems;

  ///@{
  /**
   * \brief \f$x\f$, \f$y\f$, \f$z\f$ coordinates of the nodes on the boundary
   *
   * This is ordered according to nekRS's internal geometry layout, and is indexed
   * first by the element and then by the node.
   **/
  float* _x;
  float* _y;
  float* _z;
  ///@}

  /**
   * \brief Mapping of GLL indices to MooseMesh node indices
   *
   * In nekRS, the GLL points are ordered by \f$x\f$, \f$y\f$, and \f$z\f$ coordinates,
   * but in order to construct sensible elements in Moose, we need to reorder these
   * points so that they match a libMesh-friendly node ordering. Without such a mapping,
   * we would construct triangles with zero/negative Jacobians instead of quad elements.
   * By indexing in the GLL index, this returns the node index.
   **/
  std::vector<int> _node_index;

  /**
   * \brief Mapping of MooseMesh node indices to GLL indices
   * In nekRS, the GLL points are ordered by \f$x\f$, \f$y\f$, and \f$z\f$ coordinates,
   * but in order to construct sensible elements in Moose, we need to reorder these
   * points so that they match a libMesh-friendly node ordering. Without such a mapping,
   * we would construct triangles with zero/negative Jacobians instead of quad elements.
   * By indexing in the node index, this returns the GLL index.
   **/
   std::vector<int> _gll_index;
};
