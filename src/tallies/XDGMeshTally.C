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

XDGMeshTally::XDGMeshTally(const InputParameters & parameters)
  : MeshTallyBase(parameters)
{
  if (isParamSetByUser("block"))
    paramError("block", "Cannot use subdomain restriction when using XDG mesh tallies!");

  if (_is_adaptive)
    mooseError("XDG mesh tallies do not support adaptive mesh refinement at present! Please use a 'MeshTally' instead.");

  // Gather all element types in the mesh.
  std::unordered_set<ElemType> contained_elem;
  auto begin = _openmc_problem.getMooseMesh().activeLocalElementsBegin();
  auto end = _openmc_problem.getMooseMesh().activeLocalElementsEnd();
  for (const auto & elem : libMesh::as_range(begin, end))
    contained_elem.insert(elem->type());

  // Check to make sure the mesh only contains a single element type.
  if (contained_elem.size() > 1)
    mooseError("XDG mesh tallies only support single-element-type meshes! Please "
               "ensure your mesh uses a single element type, or use a 'MeshTally' instead.");

  // Check to make sure all elements are TET4s or HEX8s.
  for (auto elem_type : contained_elem)
    if (elem_type != ElemType::TET4 && elem_type != ElemType::HEX8)
      mooseError("XDG mesh tallies only support TET4 and HEX8 elements! Either "
                 "ensure your mesh only uses either TET4 or HEX8 elements, or use a 'MeshTally' instead.");
}

unsigned int
XDGMeshTally::getOpenMCMeshBuildDoFMaps()
{
  // Create the OpenMC mesh which will be tallied on.
  if (!_mesh_template_filename)
  {
    // Build the DoF maps necessary for AMR.
    buildDoFMaps(&_openmc_problem.getMooseMesh().getMesh());

    _xdg_mesh_manager.reset(new xdg::LibMeshManager(&_openmc_problem.getMooseMesh().getMesh()));
    _xdg_mesh_manager->init();
    _xdg_mesh_manager->parse_metadata();

    _xdg_instance.reset(new xdg::XDG(_xdg_mesh_manager, xdg::RTLibrary::EMBREE));
    openmc::model::meshes.emplace_back(std::make_unique<openmc::XDGMesh>(_xdg_instance, _openmc_problem.scaling()));
  }
  else
  {
    openmc::model::meshes.emplace_back(
        std::make_unique<openmc::XDGMesh>(*_mesh_template_filename, _openmc_problem.scaling()));
  }

  return openmc::model::meshes.size() - 1;
}

#endif
