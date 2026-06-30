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

#include "OpenMCMeshGenerator.h"
#include "GeneratedMeshGenerator.h"

registerMooseObject("CardinalApp", OpenMCMeshGenerator);

InputParameters
OpenMCMeshGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();

  params.addClassDescription(
      "A class that generates a mesh from an available OpenMC regular mesh.");
  params.addRequiredParam<unsigned int>(
      "mesh_id", "The id of the OpenMC mesh that this class should provide to Cardinal.");
  params.addRangeCheckedParam<Real>(
      "scaling",
      1.0,
      "scaling > 0.0",
      "Scaling factor used to scale the OpenMC mesh dimensions for the creation of the mesh");
  params.addParam<FileName>(
      "xml_directory", "./", "The directory in which to look for OpenMC XML files.");
  return params;
}

OpenMCMeshGenerator::OpenMCMeshGenerator(const InputParameters & params)
  : MeshGenerator(params),
    _mesh_id(getParam<unsigned int>("mesh_id")),
    _scaling(getParam<Real>("scaling")),
    _xml_directory(getParam<FileName>("xml_directory"))
{
  // Check to make sure the mesh exists.
  if (openmc::model::mesh_map.count(_mesh_id) == 0)
    paramError("mesh_id",
               "A mesh with the id " + Moose::stringify(_mesh_id) +
                   " does not exist in the OpenMC model! Please make sure the mesh has been "
                   "added in the OpenMC model and you've supplied the correct mesh id.");

  _openmc_mesh_index = openmc::model::mesh_map.at(_mesh_id);
  _openmc_mesh = openmc::model::meshes[_openmc_mesh_index].get();

  if (_openmc_mesh->n_dimension_ != 3)
  {
    mooseError("At this time, this object only supports 3D meshes. Please contact the Cardinal "
               "developer team if you need 2D or 1D meshes!");
  }

  // Construct mesh using a subgenerator
  if (_openmc_mesh->get_mesh_type() == openmc::RegularMesh::mesh_type)
  {
    openmc::RegularMesh * regular_mesh = dynamic_cast<openmc::RegularMesh *>(_openmc_mesh);

    InputParameters params = GeneratedMeshGenerator::validParams();
    params.set<MooseEnum>("dim") = regular_mesh->n_dimension_;
    params.set<unsigned int>("nx") = regular_mesh->shape_[0];
    params.set<unsigned int>("ny") = regular_mesh->shape_[1];
    params.set<unsigned int>("nz") = regular_mesh->shape_[2];
    params.set<Real>("xmin") = regular_mesh->lower_left()[0] / _scaling;
    params.set<Real>("xmax") = regular_mesh->upper_right()[0] / _scaling;
    params.set<Real>("ymin") = regular_mesh->lower_left()[1] / _scaling;
    params.set<Real>("ymax") = regular_mesh->upper_right()[1] / _scaling;
    params.set<Real>("zmin") = regular_mesh->lower_left()[2] / _scaling;
    params.set<Real>("zmax") = regular_mesh->upper_right()[2] / _scaling;
    addMeshSubgenerator("GeneratedMeshGenerator", name() + "_openmc_sub_mesh", params);

    _build_mesh = &getMeshByName(name() + "_openmc_sub_mesh");
  }
  else
  {
    mooseError("Only implemented for openmc::RegularMesh!");
  }
}

std::unique_ptr<MeshBase>
OpenMCMeshGenerator::generate()
{
  return std::move(*_build_mesh);
}

#endif
