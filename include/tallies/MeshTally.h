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

  /**
   * A function which gathers the sums and means from all tallies linked to this tally. MeshTally
   * overrides this function to gather global tallies for distributed mesh tallies.
   */
  virtual void gatherLinkedSum() override;

  /**
   * Override the relaxation function for mesh tallies to handle projection for adaptivity.
   */
  virtual void relaxAndNormalizeTally() override;

  /**
   * A function to return if this object is adding a global tally. MeshTally modifies this behavior
   * to add a single global tally for distributed mesh tallies (which then communicate with
   * tally linkages).
   */
  virtual bool addingGlobalTally() const override { return _needs_global_tally && _instance == 0; }

protected:
  /**
   * A function which stores the results of this tally into the created
   * auxvariables. This implements the copy transfer between the tally mesh and the MOOSE mesh.
   * @param[in] var_numbers variables which the tally will store results in
   * @param[in] local_score index into the tally's local array of scores which represents the
   * current score being stored
   * @param[in] tally_vals the tally values to store
   * @param[in] norm_by_src_rate whether or not tally_vals should be normalized by the source rate
   * @return the sum of the tally over all bins.
   */
  virtual Real storeResultsInner(const std::vector<unsigned int> & var_numbers,
                                 unsigned int local_score,
                                 const std::vector<OMCTensor> & tally_vals,
                                 bool norm_by_src_rate = true) override;
  /**
   * Check the setup of the mesh template and translations. Because a simple copy transfer
   * is used to write a mesh tally onto the [Mesh], we require that the
   * meshes are identical - both in terms of the element ordering and the actual dimensions of
   * each element. This function performs as many checks as possible to ensure that the meshes
   * are indeed identical.
   */
  void checkMeshTemplateAndTranslations();

  /// An enum for the different cases when applying relaxation to an adaptive mesh tally.
  enum class AMRRelaxation
  {
    CaseI = 0,
    CaseII = 1,
    CaseIII = 2
  };

  /**
   * There are three cases for relaxation with AMR mesh tallies:
   * i)   A spatial bin from the previous solution and a spatial bin from the current
   *      solution correspond one-to-one.
   * ii)  A spatial bin from the previous solution maps to N spatial bins from the current
   *      solution (previous element was at a lower refinement level).
   * iii) N spatial bins from the previous solution map to a single spatial bin
   *      from the current solution (previous element was at a higher refinement
   *      level).
   *
   * This function classifies an element according to these three cases described above.
   * @param[in] current_element the element to classify
   * @return an enum corresponding to the classification
   */
  AMRRelaxation classifyRelaxationCase(const libMesh::Elem * current_element) const;

  /**
   * This function performs relaxation on the solution vectors from two different
   * Picard iterations: 'previous' and 'current_raw'. Results are saved to
   * 'current_relaxed'. As mentioned above, there are three relaxation cases:
   * Case I:   The spatial bin can be relaxed in place.
   * Case II:  The N spatial bins from the current solution need to be accumulated
   *           up to the level of the previous solution. Then, relaxation can be
   *           performed. Finally, the relaxed value can be distributed to the
   *           N current solution spatial bins according to how much that bin
   *           contributed to the integral.
   * Case III: The N spatial bins from the previous step must be accumulated down
   *           to the level of the current solution. Then, relaxation can then be
   *           performed. The relaxed value can then be used in-place.
   * @param[in] alpha the relaxation factor being applied to tally values
   * @param[in] previous the relaxed tally value from the previous iteration
   * @param[in] current_raw the raw (unrelaxed) tally value from the current iteration
   * @param[out] current_relaxed the relaxed tally value on the current iteration
   */
  void projectAndRelaxAMR(Real alpha, const OMCTensor & previous,
                          const OMCTensor & current_raw, OMCTensor & current_relaxed);

  /**
   * Determine which ancestor of 'active_elem' was active on the previous step.
   * @param[in] active_elem the element to find the active ancestor of
   * @return the previous active ancestor
   */
  const Elem * previousActiveAncestor(const Elem * active_elem) const;

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
  openmc::UnstructuredMesh * _mesh_template;

  /// Whether we're using an indirection layer to map between the OpenMC mesh tally and the MOOSE mesh.
  const bool _use_dof_map;

  /// A mapping between the OpenMC bins (active block restricted elements) and all elements.
  std::vector<dof_id_type> _bin_to_element_mapping;

  ///----------------------------------------------------------------------------///
  /// The following variables are only maintained when adaptivity is being used  ///
  /// and relaxation is requested. They are used to map between solution vectors ///
  /// in different Picard iterations to apply relaxation.                        ///
  ///----------------------------------------------------------------------------///
  /// Dual of '_bin_to_element_mapping'.
  std::vector<int64_t> _element_to_bin_mapping;

  /// The previous bin to element mapping.
  std::vector<dof_id_type> _prev_bin_to_element_mapping;

  /// The dual of '_prev_bin_to_element_mapping'.
  std::vector<int64_t> _prev_elem_to_bin_mapping;
  ///----------------------------------------------------------------------------///
};
