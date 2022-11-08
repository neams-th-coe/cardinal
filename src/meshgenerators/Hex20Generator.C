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

#include "Hex20Generator.h"
#include "CastUniquePointer.h"
#include "libmesh/mesh_tools.h"

registerMooseObject("CardinalApp", Hex20Generator);

InputParameters
Hex20Generator::validParams()
{
  InputParameters params = NekMeshGenerator::validParams();
  params.addClassDescription(
      "Converts a HEX27 mesh to a HEX20 mesh, while optionally preserving "
      "circular edges (which were faceted) in the HEX27 mesh.");
  return params;
}

Hex20Generator::Hex20Generator(const InputParameters & params)
  : NekMeshGenerator(params)
{
  _n_start_nodes = Hex27::num_nodes;
  _n_start_nodes_per_side = Hex27::nodes_per_side;

  _n_end_nodes = Hex20::num_nodes;

  // for each face, the mid-side nodes to be adjusted
  _side_ids.push_back({12, 15, 14, 13});
  _side_ids.push_back({11, 9, 17, 19});
  _side_ids.push_back({8, 18, 10, 16});
  _side_ids.push_back({9, 11, 19, 17});
  _side_ids.push_back({10, 8, 16, 18});
  _side_ids.push_back({12, 13, 14, 15});

  // corner nodes for each face
  _corner_nodes.resize(Hex27::num_sides);
  for (unsigned int i = 0; i < Hex27::num_sides; ++i)
    for (unsigned int j = 0; j < Hex8::nodes_per_side; ++j)
      _corner_nodes[i].push_back(Hex27::side_nodes_map[i][j]);

  _across_pair.resize(Hex27::num_sides);
  _across_pair[0] = {{0, 4}, {8, 16}, {1, 5}, {11, 19}, {20, 25}, {9, 17}, {3, 7}, {10, 18}, {2, 6}};
  _across_pair[1] = {{0, 3}, {8, 10}, {1, 2}, {12, 15}, {21, 23}, {13, 14}, {4, 7}, {16, 18}, {5, 6}};
  _across_pair[2] = {{1, 0}, {9, 11}, {2, 3}, {13, 12}, {22, 24}, {14, 15}, {5, 4}, {17, 19}, {6, 7}};
  _across_pair[3] = {{3, 0}, {10, 8}, {2, 1}, {15, 12}, {23, 21}, {14, 13}, {7, 4}, {18, 16}, {6, 5}};
  _across_pair[4] = {{0, 1}, {11, 9}, {3, 2}, {12, 13}, {24, 22}, {15, 14}, {4, 5}, {19, 17}, {7, 6}};
  _across_pair[5] = {{4, 0}, {16, 8}, {5, 1}, {19, 11}, {25, 20}, {17, 9}, {7, 3}, {18, 10}, {6, 2}};

  _across_face.resize(Hex27::num_sides);
  _across_face[0] = 5;
  _across_face[1] = 3;
  _across_face[2] = 4;
  _across_face[3] = 1;
  _across_face[4] = 2;
  _across_face[5] = 0;
}

std::pair<unsigned int, unsigned int>
Hex20Generator::pairedNodesAboutMidPoint(const unsigned int & node_id) const
{
  int index = node_id - Hex8::num_nodes;
  unsigned int p0 = Hex27::edge_nodes_map[index][0];
  unsigned int p1 = Hex27::edge_nodes_map[index][1];
  return {p0, p1};
}

void
Hex20Generator::checkElementType(std::unique_ptr<MeshBase> & mesh)
{
  for (const auto & elem : mesh->element_ptr_range())
  {
    libMesh::Hex27 * hex27 = dynamic_cast<libMesh::Hex27 *>(elem);
    if (!hex27)
      mooseError("This mesh generator can only be applied to HEX27 elements!");
  }
}

bool
Hex20Generator::isCornerNode(const unsigned int & node) const
{
  return node < Hex8::num_nodes;
}

const std::vector<unsigned int>
Hex20Generator::nodesOnFace(const unsigned int & face) const
{
  const auto & side_map = Hex27::side_nodes_map[face];
  std::vector<unsigned int> nodes;
  nodes.assign(std::begin(side_map), std::end(side_map));
  return nodes;
}

std::unique_ptr<MeshBase>
Hex20Generator::generate()
{
  std::unique_ptr<MeshBase> mesh = std::move(_input);
  auto & boundary_info = mesh->get_boundary_info();
  const auto & original_boundaries = boundary_info.get_boundary_ids();

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
    libMesh::Hex27 * hex27 = dynamic_cast<libMesh::Hex27 *>(elem);
    elem_ids.push_back(hex27->id());
    elem_block_ids.push_back(hex27->subdomain_id());

    for (unsigned int j = 0; j < _n_end_nodes; ++j)
      node_ids[i].push_back(hex27->node_ref(j).id());

    i++;
  }

  // move the nodes on the surface of interest
  moveNodes(mesh, polygon_layer_smoothing);

  saveAndRebuildNodes(mesh);

  // create the elements
  for (unsigned int i = 0; i < elem_ids.size(); ++i)
  {
    auto elem = new Hex20;
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
