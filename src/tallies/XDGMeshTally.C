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

#ifdef ENABLE_XDG
#include "XDGMeshTally.h"

#include "openmc/xdg.h"

registerMooseObject("CardinalApp", XDGMeshTally);

InputParameters
XDGMeshTally::validParams()
{
  auto params = MeshTallyBase::validParams();
  params.addClassDescription("A class which implements XDG unstructured mesh tallies.");

  return params;
}

XDGMeshTally::XDGMeshTally(const InputParameters & parameters) : MeshTallyBase(parameters)
{
  if (isParamSetByUser("block"))
    paramError("block", "Cannot use subdomain restriction when using XDG mesh tallies!");

  if (_is_adaptive)
    mooseError("XDG mesh tallies do not support adaptive mesh refinement at present! Please use a "
               "'MeshTally' instead.");
}

unsigned int
XDGMeshTally::getOpenMCMeshBuildDoFMaps()
{
  // Create the OpenMC mesh which will be tallied on.
  if (!_mesh_template_filename)
  {
    // Build the DoF maps necessary for AMR.
    buildDoFMaps(&_openmc_problem.getMooseMesh().getMesh());

    catchXDGError(
        [&]()
        {
          _xdg_mesh_manager.reset(
              new xdg::LibMeshManager(&_openmc_problem.getMooseMesh().getMesh()));
          _xdg_mesh_manager->init();
          _xdg_mesh_manager->parse_metadata();
        },
        "When attempting to initialize a mesh tally based on the MOOSE [Mesh], XDG reported:\n");

    _xdg_instance.reset(new xdg::XDG(_xdg_mesh_manager, xdg::RTLibrary::EMBREE));
    openmc::model::meshes.emplace_back(
        std::make_unique<openmc::XDGMesh>(_xdg_instance, _openmc_problem.scaling()));
  }
  else
  {
    catchXDGError(
        [&]()
        {
          openmc::model::meshes.emplace_back(std::make_unique<openmc::XDGMesh>(
              *_mesh_template_filename, _openmc_problem.scaling()));
        },
        "When attempting to initialize a mesh tally from a file, XDG reported:\n");
  }

  return openmc::model::meshes.size() - 1;
}

#endif
