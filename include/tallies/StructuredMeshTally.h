/********************************************************************/
/*                  SOFTWARE COPYRIGHT NOTICE                        */
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

#include "TallyBase.h"
#include "StructuredMesh.h"

#include "openmc/tallies/filter_mesh.h"

/**
 * A tally which maps results onto an OpenMC structured mesh (regular or rectilinear).
 *
 * Unlike the unstructured MeshTally, the OpenMC structured mesh and a matching native
 * libMesh mesh are both generated "on the fly" from text-based parameters, and are fully
 * independent of the mesh in the [Mesh] block. The tally results are read back and stored
 * into the generated mesh, which this object owns.
 */
class StructuredMeshTally : public TallyBase
{
public:
  static InputParameters validParams();

  StructuredMeshTally(const InputParameters & parameters);

  virtual std::pair<unsigned int, openmc::Filter *> spatialFilter() override;

  virtual void resetTally() override;

  /// Get a reference to the structured mesh (both OpenMC and libMesh representations)
  structured_mesh::StructuredMesh & structuredMesh() { return *_mesh; }

  /// Get a reference to the structured mesh (both OpenMC and libMesh representations)
  const structured_mesh::StructuredMesh & structuredMesh() const { return *_mesh; }

protected:
  virtual Real storeResultsInner(const std::vector<unsigned int> & var_numbers,
                                 unsigned int local_score,
                                 const std::vector<OMCTensor> & tally_vals,
                                 bool norm_by_src_rate = true) override;

  virtual bool ownsTallyMesh() const override { return true; }

  /// Build the per-axis node coordinates from the input parameters.
  std::array<std::vector<Real>, 3> buildCoordinates() const;

private:
  /// The structured mesh (both the OpenMC and libMesh representations).
  std::unique_ptr<structured_mesh::StructuredMesh> _mesh;

  /// The OpenMC mesh filter for this structured mesh tally.
  openmc::MeshFilter * _mesh_filter;

  /// Number of spatial dimensions.
  const unsigned int _dimension;

  /// Mesh type (regular or rectilinear).
  const structured_mesh::MeshType _mesh_type;
};