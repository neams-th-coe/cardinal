/********************************************************************/
/*                  SOFTWARE COPYRIGHT NOTICE                 */
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

#include "MooseTypes.h"

#include <array>
#include <memory>
#include <string>
#include <vector>

#include "openmc/mesh.h"

namespace libMesh
{
class ReplicatedMesh;
namespace Parallel
{
class Communicator;
}
}

namespace structured_mesh
{

/**
 * Which kind of OpenMC structured mesh to build.
 */
enum class MeshType { REGULAR, RECTILINEAR };

/**
 * A self-contained helper which wraps the construction of OpenMC's structured
 * (regular / rectilinear) meshes and the generation of a matching native libMesh
 * mesh.
 *
 * Both meshes are described by the same per-axis node coordinate lines, so the
 * OpenMC tally bins line up one-to-one with the elements of the native libMesh
 * mesh. Element/bin ordering matches openmc::StructuredMesh::get_bin_from_indices
 * (x-fastest, then y, then z).
 */
class StructuredMesh
{
public:
  /**
   * Constructor.
   * @param[in] coords node coordinates along each axis. entry i must contain
   *            coords[i].size() = (number of cells along axis i) + 1 points,
   *            strictly increasing. Axes with a single point are collapsed
   *            (i.e. do not contribute cells).
   * @param[in] dim  number of spatial dimensions (1, 2, or 3). This is the
   *         number of non-collapsed axes.
   * @param[in] style whether to use an OpenMC regular (uniform) or rectilinear
   *         (non-uniform) mesh
   */
  StructuredMesh(const std::array<std::vector<Real>, 3> & coords,
                 unsigned int dim,
                 MeshType style);

  /// Build the OpenMC structured mesh
  void buildOpenMCMesh();

  /// Build the native libMesh mesh which mirrors the OpenMC mesh
  void buildLibMeshMesh(const libMesh::Parallel::Communicator & comm);

  /// Get the OpenMC structured mesh
  openmc::StructuredMesh * openmcMesh() const { return _openmc_mesh; }

  /// Index into openmc::model::meshes of the mesh built by this object
  unsigned int meshIndex() const { return _mesh_index; }

  /// Get the native libMesh mesh
  libMesh::ReplicatedMesh & libmeshMesh() { return *_libmesh_mesh; }
  const libMesh::ReplicatedMesh & libmeshMesh() const { return *_libmesh_mesh; }

  /// Get the native libMesh mesh
  void setID(int32_t id = -1);

  /// Number of mesh bins / elements
  unsigned int nBins() const { return _n_bins; }

  /// Number of spatial dimensions
  unsigned int dim() const { return _dim; }

  /// Number of cells along each axis (collapsed axes have 1)
  const std::array<unsigned int, 3> & shape() const { return _shape; }

  /// The OpenMC volume [cm^3] of a bin
  Real binVolume(unsigned int bin) const;

  /// The OpenMC centroid of a bin
  Point binCentroid(unsigned int bin) const;

  /**
   * Add a scalar, per-element field in which to store results (one entry per bin).
   * @param[in] name field name
   * @return index of the newly-added field
   */
  unsigned int addScalarField(const std::string & name);

  /// Number of scalar fields added
  unsigned int nScalarFields() const { return _fields.size(); }

  /// Get all scalar field names
  const std::vector<std::string> & fieldNames() const { return _field_names; }

  /// Set the per-bin values for a field
  void setBinValues(unsigned int field, const std::vector<Real> & values);

  /// Get the per-bin values for a field
  const std::vector<Real> & getBinValues(unsigned int field) const;

private:
  /// Build the OpenMC regular mesh
  void buildRegular();

  /// Build the OpenMC rectilinear mesh
  void buildRectilinear();

  /// Build node coordinates along each axis
  std::array<std::vector<Real>, 3> _coords;

  /// Number of spatial dimensions
  unsigned int _dim;

  /// Number of bins along each axis, set by the constructor
  std::array<unsigned int, 3> _shape;

  /// The OpenMC mesh type
  MeshType _style;

  /// Total number of mesh bins
  unsigned int _n_bins;

  /// Index of this mesh in openmc::model::meshes
  unsigned int _mesh_index;

  /// The OpenMC structured mesh (owned by openmc::model::meshes)
  openmc::StructuredMesh * _openmc_mesh = nullptr;

  /// The native libMesh mesh
  std::unique_ptr<libMesh::ReplicatedMesh> _libmesh_mesh;

  /// Per-bin fields holding the extracted tally results
  std::vector<std::string> _field_names;
  std::vector<std::vector<Real>> _fields;
};
} // namespace structured_mesh