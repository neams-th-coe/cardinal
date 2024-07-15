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
  if (isParamValid("tally_estimator"))
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

  /**
   * If the instance isn't zero this variable is a translated mesh tally. It will accumulate it's
   * scores in a different set of variables (the auxvars which are added by the first tally in a
   * sequence of mesh tallies), and so it doesn't need to create any auxvars.
   */
  if (_instance != 0)
    _tally_name = std::vector<std::string>();
}

void
MeshTally::initializeTally()
{
  // Clear cached results.
  _local_sum_tally.clear();
  _local_sum_tally.resize(_tally_score.size(), 0.0);
  _local_mean_tally.clear();
  _local_mean_tally.resize(_tally_score.size(), 0.0);

  _current_tally.resize(_tally_score.size());
  _current_raw_tally.resize(_tally_score.size());
  _current_raw_tally_std_dev.resize(_tally_score.size());
  _previous_tally.resize(_tally_score.size());

  // Create the mesh filter and the mesh.
  _filter_index = openmc::model::tally_filters.size();
  _mesh_filter = makeMeshFilter();

  // TODO: Append to this to add an energy filter.
  std::vector<openmc::Filter *> filters = {_mesh_filter};

  // Create the tally, assign the required filters and apply the triggers.
  _local_tally_index = openmc::model::tallies.size();
  _local_tally = openmc::Tally::create();
  _local_tally->set_scores(_tally_score);
  _local_tally->estimator_ = _estimator;
  _local_tally->set_filters(filters);
  applyTriggersToLocalTally(_local_tally);

  // Validate the mesh filters to make sure we can run a copy transfer to the [Mesh].
  checkMeshTemplateAndTranslations();
}

void
MeshTally::resetTally()
{
  // Erase the tally.
  openmc::model::tallies.erase(openmc::model::tallies.begin() + _local_tally_index);

  // Erase the filter(s).
  openmc::model::tally_filters.erase(openmc::model::tally_filters.begin() + _filter_index);

  // Erase the OpenMC mesh.
  openmc::model::meshes.erase(openmc::model::meshes.begin() + _mesh_index);
}

Real
MeshTally::storeResults(const std::vector<unsigned int> & var_numbers,
                        unsigned int local_score,
                        unsigned int global_score)
{
  Real total = 0.0;

  unsigned int offset = _instance * _mesh_filter->n_bins();
  for (decltype(_mesh_filter->n_bins()) e = 0; e < _mesh_filter->n_bins(); ++e)
  {
    Real power_fraction = _current_tally[local_score](e);

    // divide each tally by the volume that it corresponds to in MOOSE
    // because we will apply it as a volumetric tally (per unit volume).
    // Because we require that the mesh template has units of cm based on the
    // mesh constructors in OpenMC, we need to adjust the division
    Real volumetric_power = power_fraction * _openmc_problem.tallyMultiplier(global_score) /
                            _mesh_template->volume(e) * _openmc_problem.scaling() *
                            _openmc_problem.scaling() * _openmc_problem.scaling();
    total += power_fraction;

    std::vector<unsigned int> elem_ids = {offset + e};
    fillElementalAuxVariable(var_numbers[local_score], elem_ids, volumetric_power);
  }

  return total;
}

void
MeshTally::storeExternalResults(const std::vector<unsigned int> & ext_var_numbers,
                                unsigned int local_score,
                                unsigned int global_score,
                                const std::string & output_type)
{
  unsigned int offset = _instance * _mesh_filter->n_bins();
  if (output_type == "unrelaxed_tally_std_dev")
  {
    for (decltype(_mesh_filter->n_bins()) e = 0; e < _mesh_filter->n_bins(); ++e)
    {
      Real power_fraction = _current_raw_tally_std_dev[local_score](e);

      // divide each tally by the volume that it corresponds to in MOOSE
      // because we will apply it as a volumetric tally (per unit volume).
      // Because we require that the mesh template has units of cm based on the
      // mesh constructors in OpenMC, we need to adjust the division
      Real volumetric_power = power_fraction * _openmc_problem.tallyMultiplier(global_score) /
                              _mesh_template->volume(e) * _openmc_problem.scaling() *
                              _openmc_problem.scaling() * _openmc_problem.scaling();

      std::vector<unsigned int> elem_ids = {offset + e};
      fillElementalAuxVariable(ext_var_numbers[local_score], elem_ids, volumetric_power);
    }
  }
  else if (output_type == "unrelaxed_tally")
  {
    for (decltype(_mesh_filter->n_bins()) e = 0; e < _mesh_filter->n_bins(); ++e)
    {
      Real power_fraction = _current_raw_tally[local_score](e);

      // divide each tally by the volume that it corresponds to in MOOSE
      // because we will apply it as a volumetric tally (per unit volume).
      // Because we require that the mesh template has units of cm based on the
      // mesh constructors in OpenMC, we need to adjust the division
      Real volumetric_power = power_fraction * _openmc_problem.tallyMultiplier(global_score) /
                              _mesh_template->volume(e) * _openmc_problem.scaling() *
                              _openmc_problem.scaling() * _openmc_problem.scaling();

      std::vector<unsigned int> elem_ids = {offset + e};
      fillElementalAuxVariable(ext_var_numbers[local_score], elem_ids, volumetric_power);
    }
  }
  else
    mooseError("Unknown external output " + output_type);
}

openmc::MeshFilter *
MeshTally::makeMeshFilter()
{
  // Create the OpenMC mesh which will be tallied on.
  std::unique_ptr<openmc::LibMesh> tally_mesh;
  if (!_mesh_template_filename)
    tally_mesh = std::make_unique<openmc::LibMesh>(_mesh.getMesh(), _openmc_problem.scaling());
  else
    tally_mesh =
        std::make_unique<openmc::LibMesh>(*_mesh_template_filename, _openmc_problem.scaling());

  // by setting the ID to -1, OpenMC will automatically detect the next available ID
  tally_mesh->set_id(-1);
  tally_mesh->output_ = false;
  _mesh_template = tally_mesh.get();

  // Create the mesh filter itself.
  _mesh_index = openmc::model::meshes.size();
  openmc::model::meshes.push_back(std::move(tally_mesh));

  auto filter = dynamic_cast<openmc::MeshFilter *>(openmc::Filter::create("mesh"));
  filter->set_mesh(_mesh_index);
  filter->set_translation({_mesh_translation(0), _mesh_translation(1), _mesh_translation(2)});

  return filter;
}

void
MeshTally::checkMeshTemplateAndTranslations() const
{
  // we can do some rudimentary checking on the mesh template by comparing the centroid
  // coordinates compared to centroids in the [Mesh] (because right now, we just doing a simple
  // copy transfer that necessitates the meshes to have the same elements in the same order). In
  // other words, you might have two meshes that represent the same geometry, the element ordering
  // could be different.
  unsigned int offset = _instance * _mesh_filter->n_bins();
  for (int e = 0; e < _mesh_filter->n_bins(); ++e)
  {
    auto elem_ptr = _mesh.queryElemPtr(offset + e);

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
          "Centroid for element " + Moose::stringify(offset + e) + " in the [Mesh] (cm): " +
          _openmc_problem.printPoint(centroid_mesh) + "\ndoes not match centroid for element " +
          Moose::stringify(e) + " in the 'mesh_template' with instance " +
          Moose::stringify(_instance) + " (cm): " + _openmc_problem.printPoint(centroid_template) +
          "!\n\nThe copy transfer requires that the [Mesh] and 'mesh_template' be identical.");
  }
}
#endif
