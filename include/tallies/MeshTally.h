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

class MeshTally : public TallyBase
{
public:
  static InputParameters validParams();

  MeshTally(const InputParameters & parameters);

  /// A function to initialize the tally.
  virtual void initializeTally() override;

  /// A function to reset the tally.
  virtual void resetTally() override;

  /**
   * A function which stores the results of this tally into the created auxvariables.
   * @param[in] var_numbers variables which the tally will store results in
   * @param[in] local_score index into the tally's local array of scores which represents the
   * current score being stored
   * @param[in] global_score index into the global array of tally results which represents the
   * current score being stored
   * @return the sum of the score across all tally bins
   */
  virtual Real storeResults(const std::vector<unsigned int> & var_numbers,
                            unsigned int local_score,
                            unsigned int global_score) override;

  /**
   * A function which stores the external variable results of this tally into the created
   * auxvariables.
   * @param[in] ext_var_numbers variables which the tally will store results in
   * @param[in] local_score index into the tally's local array of scores which represents the
   * current score being stored
   * @param[in] global_score index into the global array of tally results which represents the
   * current score being stored
   * @param[in] output_type the output type
   */
  virtual void storeExternalResults(const std::vector<unsigned int> & ext_var_numbers,
                                    unsigned int local_score,
                                    unsigned int global_score,
                                    const std::string & output_type) override;

protected:
  /**
   * Create the mesh filter for tallies automatically constructed by Cardinal.
   * @return the mesh filter
   */
  openmc::MeshFilter * makeMeshFilter();

  /**
   * Check the setup of the mesh template and translations. Because a simple copy transfer
   * is used to write a mesh tally onto the [Mesh], we require that the
   * meshes are identical - both in terms of the element ordering and the actual dimensions of
   * each element. This function performs as many checks as possible to ensure that the meshes
   * are indeed identical.
   */
  void checkMeshTemplateAndTranslations() const;

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
  unsigned int _instance;

  /// The index of the OpenMC tally this object wraps.
  unsigned int _local_tally_index;

  /// The index of the first filter added by this tally.
  unsigned int _filter_index;

  /// The index of the mesh added by this tally.
  unsigned int _mesh_index;

  /// OpenMC mesh filter for this unstructured mesh tally.
  openmc::MeshFilter * _mesh_filter;

  /// OpenMC unstructured mesh instance for use with mesh tallies
  const openmc::LibMesh * _mesh_template;
};
