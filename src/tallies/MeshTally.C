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

#ifdef ENABLE_OPENMC_COUPLING
#include "MeshTally.h"

#include "libmesh/replicated_mesh.h"

registerMooseObject("CardinalApp", MeshTally);

InputParameters
MeshTally::validParams()
{
  auto params = TallyBase::validParams();
  params.addClassDescription("A class which implements unstructured mesh tallies.");
  params.addParam<std::string>("mesh_template",
                               "Mesh tally template for OpenMC when using mesh tallies; "
                               "at present, this mesh must exactly match the mesh used in the "
                               "[Mesh] block because a one-to-one copy "
                               "is used to get OpenMC's tally results on the [Mesh].");
  params.addParam<Point>("mesh_translation",
                         "Coordinate to which this mesh should be "
                         "translated. Units must match those used to define the [Mesh].");

  // The index of this tally into an array of mesh translations. Defaults to zero.
  params.addPrivateParam<unsigned int>("instance", 0);

  return params;
}

MeshTally::MeshTally(const InputParameters & parameters)
  : TallyBase(parameters),
    _mesh_translation(isParamValid("mesh_translation") ? getParam<Point>("mesh_translation")
                                                       : Point(0.0, 0.0, 0.0)),
    _instance(getParam<unsigned int>("instance"))
{
  // Error check the estimators.
  if (isParamValid("estimator"))
  {
    if (_estimator == openmc::TallyEstimator::TRACKLENGTH)
      mooseError("Tracklength estimators are currently incompatible with mesh tallies!");
  }
  else
    _estimator = openmc::TallyEstimator::COLLISION;

  // Error check the mesh template.
  if (_mesh.getMesh().allow_renumbering() && !_mesh.getMesh().is_replicated())
    mooseError(
        "Mesh tallies currently require 'allow_renumbering = false' to be set in the [Mesh]!");

  if (isParamValid("mesh_template"))
    _mesh_template_filename = &getParam<std::string>("mesh_template");
  else
  {
    if (std::abs(_openmc_problem.scaling() - 1.0) > 1e-6)
      mooseError("Directly tallying on the [Mesh] is only supported for 'scaling' of unity. "
                 "Instead, please make a file containing your tally mesh and set it with "
                 "'mesh_template'. You can generate a mesh file corresponding to the [Mesh] "
                 "by running:\n\ncardinal-opt -i " +
                 _app.getFileName() + " --mesh-only");

    // for distributed meshes, each rank only owns a portion of the mesh information, but
    // OpenMC wants the entire mesh to be available on every rank. We might be able to add
    // this feature in the future, but will need to investigate
    if (!_mesh.getMesh().is_replicated())
      mooseError("Directly tallying on the [Mesh] block by OpenMC is not yet supported "
                 "for distributed meshes!");

    if (isParamValid("mesh_translation"))
      mooseError("The mesh filter cannot be translated if directly tallying on the mesh "
                 "provided in the [Mesh] block!");
  }

  if (_mesh_template_filename && _is_adaptive)
    mooseError("Adaptivity is only supported when tallying on the mesh in the [Mesh] block!");

  /**
   * If the instance isn't zero this variable is a translated mesh tally. It will accumulate it's
   * scores in a different set of variables (the auxvars which are added by the first tally in a
   * sequence of mesh tallies), and so it doesn't need to create any auxvars.
   */
  if (_instance != 0)
    _tally_name = std::vector<std::string>();
}

std::pair<unsigned int, openmc::Filter *>
MeshTally::spatialFilter()
{
  // Create the OpenMC mesh which will be tallied on.
  if (!_mesh_template_filename)
  {
    if (_is_adaptive)
    {
      /**
       * Need to create a copy of the mesh which only contains active elements. If this isn't
       * done, the equation system added by the OpenMC mesh object will throw an error during
       * the refinement / coarsening process as it has no idea that AMR is required.
       */
      _libmesh_mesh_copy =
          std::make_unique<libMesh::ReplicatedMesh>(_openmc_problem.comm(), _mesh.dimension());

      auto msh = dynamic_cast<const libMesh::ReplicatedMesh *>(_mesh.getMeshPtr());
      if (!msh)
        mooseError("Internal error: The mesh is not a replicated mesh.");

      msh->create_submesh(
          *_libmesh_mesh_copy.get(), msh->active_elements_begin(), msh->active_elements_end());
      _libmesh_mesh_copy->allow_find_neighbors(true);
      _libmesh_mesh_copy->allow_renumbering(false);
      _libmesh_mesh_copy->prepare_for_use();

      _active_to_total_mapping.clear();
      _active_to_total_mapping.reserve(msh->n_active_elem());
      for (const auto & old_elem :
           libMesh::as_range(msh->active_elements_begin(), msh->active_elements_end()))
        _active_to_total_mapping.push_back(old_elem->id());

      openmc::model::meshes.emplace_back(
          std::make_unique<openmc::LibMesh>(*_libmesh_mesh_copy.get(), _openmc_problem.scaling()));
    }
    else
      openmc::model::meshes.emplace_back(
          std::make_unique<openmc::LibMesh>(_mesh.getMesh(), _openmc_problem.scaling()));
  }
  else
    openmc::model::meshes.emplace_back(
        std::make_unique<openmc::LibMesh>(*_mesh_template_filename, _openmc_problem.scaling()));

  _mesh_index = openmc::model::meshes.size() - 1;
  _mesh_template = dynamic_cast<openmc::LibMesh *>(openmc::model::meshes[_mesh_index].get());

  // by setting the ID to -1, OpenMC will automatically detect the next available ID
  _mesh_template->set_id(-1);
  _mesh_template->output_ = false;

  _mesh_filter = dynamic_cast<openmc::MeshFilter *>(openmc::Filter::create("mesh"));
  _mesh_filter->set_mesh(_mesh_index);
  _mesh_filter->set_translation({_mesh_translation(0), _mesh_translation(1), _mesh_translation(2)});

  // Validate the mesh filters to make sure we can run a copy transfer to the [Mesh].
  checkMeshTemplateAndTranslations();

  return std::make_pair(openmc::model::tally_filters.size() - 1, _mesh_filter);
}

void
MeshTally::resetTally()
{
  TallyBase::resetTally();

  // Erase the OpenMC mesh.
  openmc::model::meshes.erase(openmc::model::meshes.begin() + _mesh_index);
}

Real
MeshTally::storeResultsInner(const std::vector<unsigned int> & var_numbers,
                             unsigned int local_score,
                             unsigned int global_score,
                             std::vector<xt::xtensor<double, 1>> tally_vals,
                             bool norm_by_src_rate)
{
  Real total = 0.0;

  unsigned int mesh_offset = _instance * _mesh_filter->n_bins();
  for (unsigned int ext_bin = 0; ext_bin < _num_ext_filter_bins; ++ext_bin)
  {
    for (decltype(_mesh_filter->n_bins()) e = 0; e < _mesh_filter->n_bins(); ++e)
    {
      Real power_fraction = tally_vals[local_score](ext_bin * _mesh_filter->n_bins() + e);

      // divide each tally by the volume that it corresponds to in MOOSE
      // because we will apply it as a volumetric tally (per unit volume).
      // Because we require that the mesh template has units of cm based on the
      // mesh constructors in OpenMC, we need to adjust the division
      Real volumetric_power = power_fraction;
      volumetric_power *= norm_by_src_rate
                              ? _openmc_problem.tallyMultiplier(global_score) /
                                    _mesh_template->volume(e) * _openmc_problem.scaling() *
                                    _openmc_problem.scaling() * _openmc_problem.scaling()
                              : 1.0;
      total += power_fraction;

      auto var = var_numbers[_num_ext_filter_bins * local_score + ext_bin];
      auto elem_id = _is_adaptive ? _active_to_total_mapping[e] : mesh_offset + e;
      fillElementalAuxVariable(var, {elem_id}, volumetric_power);
    }
  }

  return total;
}

void
MeshTally::checkMeshTemplateAndTranslations() const
{
  // we can do some rudimentary checking on the mesh template by comparing the centroid
  // coordinates compared to centroids in the [Mesh] (because right now, we just doing a simple
  // copy transfer that necessitates the meshes to have the same elements in the same order). In
  // other words, you might have two meshes that represent the same geometry, the element ordering
  // could be different.
  unsigned int mesh_offset = _instance * _mesh_filter->n_bins();
  for (int e = 0; e < _mesh_filter->n_bins(); ++e)
  {
    auto elem_id = _is_adaptive ? _active_to_total_mapping[e] : mesh_offset + e;
    auto elem_ptr = _mesh.queryElemPtr(elem_id);

    // if element is not on this part of the distributed mesh, skip it
    if (!elem_ptr)
      continue;

    const auto pt = _mesh_template->centroid(e);
    Point centroid_template = {pt[0], pt[1], pt[2]};

    // The translation applied in OpenMC isn't actually registered in the mesh itself;
    // it is always added on to the point, so we need to do the same here
    centroid_template += _mesh_translation;

    // because the mesh template and [Mesh] may be in different units, we need
    // to adjust the [Mesh] by the scaling factor before doing a comparison.
    Point centroid_mesh = elem_ptr->vertex_average() * _openmc_problem.scaling();

    // if the centroids are the same except for a factor of 'scaling', then we can
    // guess that the mesh_template is probably not in units of centimeters
    if (_openmc_problem.hasScaling())
    {
      // if scaling was applied correctly, then each calculation of 'scaling' here should equal 1.
      // Otherwise, if they're all the same, then 'scaling_x' is probably the factor by which the
      // mesh_template needs to be multiplied, so we can print a helpful error message
      bool incorrect_scaling = true;
      for (unsigned int j = 0; j < OpenMCCellAverageProblem::DIMENSION; ++j)
      {
        Real scaling = centroid_mesh(j) / centroid_template(j);
        incorrect_scaling = incorrect_scaling && !MooseUtils::absoluteFuzzyEqual(scaling, 1.0);
      }

      if (incorrect_scaling)
        mooseError("The centroids of the 'mesh_template' differ from the "
                   "centroids of the [Mesh] by a factor of " +
                   Moose::stringify(centroid_mesh(0) / centroid_template(0)) +
                   ".\nDid you forget that the 'mesh_template' must be in "
                   "the same units as the [Mesh]?");
    }

    // check if centroids are the same
    bool different_centroids = false;
    for (unsigned int j = 0; j < OpenMCCellAverageProblem::DIMENSION; ++j)
      different_centroids = different_centroids ||
                            !MooseUtils::absoluteFuzzyEqual(centroid_mesh(j), centroid_template(j));

    if (different_centroids)
      mooseError(
          "Centroid for element " + Moose::stringify(elem_id) + " in the [Mesh] (cm): " +
          _openmc_problem.printPoint(centroid_mesh) + "\ndoes not match centroid for element " +
          Moose::stringify(e) + " in the 'mesh_template' with instance " +
          Moose::stringify(_instance) + " (cm): " + _openmc_problem.printPoint(centroid_template) +
          "!\n\nThe copy transfer requires that the [Mesh] and 'mesh_template' be identical.");
  }
}
#endif
