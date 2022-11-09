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
  _n_start_nodes = Quad9::num_nodes;
  _n_start_nodes_per_side = Quad9::nodes_per_side;
  _n_end_nodes = Quad8::num_nodes;
  _n_sides = Quad9::num_sides;

  // for each face, the mid-side nodes to be adjusted
  _side_ids.push_back({7, 5});
  _side_ids.push_back({4, 6});
  _side_ids.push_back({5, 7});
  _side_ids.push_back({4, 6});

  // corner nodes for each face
  _corner_nodes.resize(Quad9::num_sides);
  for (unsigned int i = 0; i < Quad9::num_sides; ++i)
    for (unsigned int j = 0; j < Quad4::nodes_per_side; ++j)
      _corner_nodes[i].push_back(Quad9::side_nodes_map[i][j]);

  _across_pair.resize(Quad9::num_sides);
  _across_pair[0] = {{0, 3}, {4, 6}, {1, 2}};
  _across_pair[1] = {{1, 0}, {5, 7}, {2, 3}};
  _across_pair[2] = {{2, 1}, {6, 4}, {3, 0}};
  _across_pair[3] = {{3, 2}, {7, 5}, {0, 1}};

  _across_face.resize(Quad9::num_sides);
  _across_face[0] = 2;
  _across_face[1] = 3;
  _across_face[2] = 0;
  _across_face[3] = 1;
}

std::pair<unsigned int, unsigned int>
Quad8Generator::pairedNodesAboutMidPoint(const unsigned int & node_id) const
{
  int index = node_id - Quad4::num_nodes;
  unsigned int p0 = Quad9::side_nodes_map[index][0];
  unsigned int p1 = Quad9::side_nodes_map[index][1];
  return {p0, p1};
}

void
Quad8Generator::checkElementType(std::unique_ptr<MeshBase> & mesh)
{
  for (const auto & elem : mesh->element_ptr_range())
  {
    libMesh::Quad9 * quad9 = dynamic_cast<libMesh::Quad9 *>(elem);
    if (!quad9)
      mooseError("This mesh generator can only be applied to QUAD9 elements!");
  }
}

bool
Quad8Generator::isCornerNode(const unsigned int & node) const
{
  return node < Quad4::num_nodes;
}

const std::vector<unsigned int>
Quad8Generator::nodesOnFace(const unsigned int & face) const
{
  const auto & side_map = Quad9::side_nodes_map[face];
  std::vector<unsigned int> nodes;
  nodes.assign(std::begin(side_map), std::end(side_map));
  return nodes;
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
