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
  auto params = MeshTallyBase::validParams();
  params.addClassDescription("A class which implements libMesh unstructured mesh tallies.");

  return params;
}

MeshTally::MeshTally(const InputParameters & parameters)
  : MeshTallyBase(parameters)
{
  bool nu_scatter =
      std::find(_tally_score.begin(), _tally_score.end(), "nu-scatter") != _tally_score.end();

  // Error check the estimators.
  if (isParamValid("estimator"))
  {
    if (_estimator == openmc::TallyEstimator::TRACKLENGTH)
      paramError("estimator",
                 "Tracklength estimators are currently incompatible with libMesh unstructured mesh tallies!");
  }
  else
    _estimator = nu_scatter ? openmc::TallyEstimator::ANALOG : openmc::TallyEstimator::COLLISION;

  // The random ray solver requires tracklength estimators, which libMesh unstructured meshes
  // don't support.
  if (_openmc_problem.runRandomRay())
    mooseError("Unstructured mesh tallies are not supported when using the random ray solver!");
}


unsigned int
MeshTally::getOpenMCMeshBuildDoFMaps()
{
  // Create the OpenMC mesh which will be tallied on.
  if (!_mesh_template_filename)
  {
    // Build the DoF maps necessary for AMR.
    buildDoFMaps(&_openmc_problem.getMooseMesh().getMesh());

    openmc::model::meshes.emplace_back(std::make_unique<openmc::AdaptiveLibMesh>(
        _openmc_problem.getMooseMesh().getMesh(), _openmc_problem.scaling(), _tally_blocks));
  }
  else
  {
    openmc::model::meshes.emplace_back(
      std::make_unique<openmc::LibMesh>(*_mesh_template_filename, _openmc_problem.scaling()));
  }

  return openmc::model::meshes.size() - 1;
}

#endif
