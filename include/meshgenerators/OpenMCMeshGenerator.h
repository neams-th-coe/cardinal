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

#include "MeshGenerator.h"

#include "openmc/mesh.h"

/*
 * MeshGenerator that uses an existing OpenMC mesh to create an equivalent one with libmesh.
 */
class OpenMCMeshGenerator : public MeshGenerator
{
public:
  static InputParameters validParams();

  OpenMCMeshGenerator(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

private:
  /// Generated mesh
  std::unique_ptr<MeshBase> * _build_mesh;

  const unsigned int _mesh_id;

  const Real & _scaling;

  openmc::Mesh * _openmc_mesh = nullptr;

  unsigned int _openmc_mesh_index;

  /// Directory in which OpenMC settings xml files are located
  const std::string & _xml_directory;
};
