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

#include "TallyBase.h"
#include "OpenMCCellAverageProblem.h"

#include "openmc/tallies/filter_mesh.h"

namespace libMesh
{
class ReplicatedMesh;
}

class MeshTally : public TallyBase
{
public:
  static InputParameters validParams();

  MeshTally(const InputParameters & parameters);

  /**
   * A function to generate the mesh filter needed by this object.
   * @return a pair where the first entry is the filter index in the global filter array and the
   * second entry is an OpenMC unstructured mesh filter
   */
  virtual std::pair<unsigned int, openmc::Filter *> spatialFilter() override;

  /// A function to reset the tally. MeshTally overrides this function to delete the OpenMC mesh.
  virtual void resetTally() override;

protected:
  /**
   * A function which stores the results of this tally into the created
   * auxvariables. This implements the copy transfer between the tally mesh and the MOOSE mesh.
   * @param[in] var_numbers variables which the tally will store results in
   * @param[in] local_score index into the tally's local array of scores which represents the
   * current score being stored
   * @param[in] global_score index into the global array of tally results which represents the
   * current score being stored
   * @param[in] tally_vals the tally values to store
   * @param[in] norm_by_src_rate whether or not tally_vals should be normalized by the source rate
   * @return the sum of the tally over all bins.
   */
  virtual Real storeResultsInner(const std::vector<unsigned int> & var_numbers,
                                 unsigned int local_score,
                                 unsigned int global_score,
                                 std::vector<xt::xtensor<double, 1>> tally_vals,
                                 bool norm_by_src_rate = true) override;
  /**
   * Check the setup of the mesh template and translations. Because a simple copy transfer
   * is used to write a mesh tally onto the [Mesh], we require that the
   * meshes are identical - both in terms of the element ordering and the actual dimensions of
   * each element. This function performs as many checks as possible to ensure that the meshes
   * are indeed identical.
   */
  void checkMeshTemplateAndTranslations();

  /**
   * Mesh template file to use for creating mesh tallies in OpenMC; currently, this mesh
   * must be identical to the mesh used in the [Mesh] block because a simple copy transfer
   * is used to extract the tallies and put on the application's mesh in preparation for
   * a transfer to another MOOSE app. If not set, this indicates that tallying will be
   * performed directly on the [Mesh].
   * TODO: allow the mesh to not be identical, both in terms of using different units
   * and more general differences like not having a particular phase present
   */
  const std::string * _mesh_template_filename = nullptr;

  /// The translation to apply to the mesh template.
  Point _mesh_translation;

  /// The index into an array of mesh translations.
  const unsigned int _instance;

  /// The index of the mesh added by this tally.
  unsigned int _mesh_index;

  /// OpenMC mesh filter for this unstructured mesh tally.
  openmc::MeshFilter * _mesh_filter;

  /// OpenMC unstructured mesh instance for use with mesh tallies
  openmc::LibMesh * _mesh_template;

  /// Blocks for which to add mesh tallies.
  std::set<SubdomainID> _tally_blocks;

  /// Whether we're using an indirection layer to map between the OpenMC mesh tally and the MOOSE mesh.
  const bool _use_dof_map;

  /**
   * For use with block restriction only. A copy of the mesh is made which only contains elements in
   * the blocks the user wishes to tally on. This is necessary at the moment as the point locators
   * used in OpenMC to find collision sites are not passed a set of block IDs to filter elements.
   * TODO: Fix this in OpenMC
   */
  std::unique_ptr<libMesh::ReplicatedMesh> _libmesh_mesh_copy;
  /// A mapping between the OpenMC bins (active block restricted elements) and all elements.
  std::vector<unsigned int> _bin_to_element_mapping;
};
