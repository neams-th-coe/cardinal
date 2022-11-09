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

#include "Quad8Generator.h"
#include "CastUniquePointer.h"
#include "libmesh/mesh_tools.h"

registerMooseObject("CardinalApp", Quad8Generator);

InputParameters
Quad8Generator::validParams()
{
  InputParameters params = NekMeshGenerator::validParams();
  params.addClassDescription(
      "Converts a Quad9 mesh to a Quad8 mesh, while optionally preserving "
      "circular edges (which were faceted) in the Quad8 mesh.");
  return params;
}

Quad8Generator::Quad8Generator(const InputParameters & params)
  : NekMeshGenerator(params)
{
}

std::pair<unsigned int, unsigned int>
Quad8Generator::pairedNodesAboutMidPoint(const unsigned int & node_id) const
{
  int index = node_id - Quad4::num_nodes;
  unsigned int p0 = Quad9::side_nodes_map[index][0];
  unsigned int p1 = Quad9::side_nodes_map[index][1];
  return {p0, p1};
}

std::unique_ptr<MeshBase>
Quad8Generator::generate()
{
  std::unique_ptr<MeshBase> mesh = std::move(_input);

  // TODO: no real reason for this restriction, just didn't need it in the first pass
  if (!mesh->is_replicated())
    mooseError("This mesh generator does not yet support distributed mesh implementations!");

  // get the boundary movement information, and check for valid user specifications
  getCircularSidesetInfo(mesh);

  // get information related to moving corners
  std::vector<Real> polygon_layer_smoothing = getPolygonSmoothingInfo(mesh);

  // get information on which boundaries to rebuild, and check for valid user specifications
  getBoundariesToRebuild(mesh);

  // check for valid element type
  checkElementType(mesh);

  initializeElemData(mesh);

  // store all information from the incoming mesh that is needed to rebuild it from scratch
  std::vector<dof_id_type> boundary_elem_ids;
  std::vector<unsigned int> boundary_face_ids;
  std::vector<std::vector<boundary_id_type>> boundary_ids;
  std::vector<std::vector<dof_id_type>> node_ids;
  std::vector<dof_id_type> elem_ids;
  std::vector<subdomain_id_type> elem_block_ids;
  node_ids.resize(mesh->n_elem());

  storeMeshInfo(mesh, boundary_elem_ids, boundary_face_ids, boundary_ids);

  int i = 0;
  for (auto & elem : mesh->element_ptr_range())
  {
    // store information about the elements
    libMesh::Quad9 * quad9 = dynamic_cast<libMesh::Quad9 *>(elem);
    elem_ids.push_back(quad9->id());
    elem_block_ids.push_back(quad9->subdomain_id());

    for (unsigned int j = 0; j < _n_end_nodes; ++j)
      node_ids[i].push_back(quad9->node_ref(j).id());

    i++;
  }

  // move the nodes on the surface of interest
  moveNodes(mesh, polygon_layer_smoothing);

  saveAndRebuildNodes(mesh);

  // create the elements
  for (unsigned int i = 0; i < elem_ids.size(); ++i)
  {
    auto elem = new Quad8;
    elem->set_id(elem_ids[i]);
    elem->subdomain_id() = elem_block_ids[i];
    mesh->add_elem(elem);

    const auto & ids = node_ids[i];
    for (unsigned int n = 0; n < ids.size(); ++n)
    {
      auto node_ptr = mesh->node_ptr(ids[n]);
      elem->set_node(n) = node_ptr;
    }
  }

  addSidesets(mesh, boundary_elem_ids, boundary_face_ids, boundary_ids);

  mesh->prepare_for_use();

  return dynamic_pointer_cast<MeshBase>(mesh);
}
