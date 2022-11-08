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
#include "UserErrorChecking.h"
#include "MooseMeshUtils.h"
#include "DelimitedFileReader.h"
#include "GeometryUtility.h"

#include "libmesh/mesh_tools.h"
#include "libmesh/cell_hex20.h"
#include "libmesh/cell_hex8.h"

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

unsigned int
Hex20Generator::getFaceNode(const unsigned int & primary_face) const
{
  const auto face_nodes = nodesOnFace(primary_face);
  return face_nodes[Hex27::nodes_per_side - 1];
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
  std::vector<Node> original_nodes;
  std::vector<dof_id_type> original_node_ids;

  // [1] store boundary ID <-> name mapping
  for (const auto & b: original_boundaries)
    _boundary_id_to_name[b] = boundary_info.get_sideset_name(b);

  int i = 0;
  for (auto & elem : mesh->element_ptr_range())
  {
    // [2] store information about the element faces
    for (unsigned short int s = 0; s < elem->n_faces(); ++s)
    {
      std::vector<boundary_id_type> b;
      boundary_info.boundary_ids(elem, s, b);

      boundary_elem_ids.push_back(elem->id());
      boundary_face_ids.push_back(s);
      boundary_ids.push_back(b);
    }

    // [3] store information about the elements
    libMesh::Hex27 * hex27 = dynamic_cast<libMesh::Hex27 *>(elem);
    elem_ids.push_back(hex27->id());
    elem_block_ids.push_back(hex27->subdomain_id());

    for (unsigned int j = 0; j < _n_end_nodes; ++j)
      node_ids[i].push_back(hex27->node_ref(j).id());

    i++;
  }

  // move the nodes on the surface of interest
  for (auto & elem : mesh->element_ptr_range())
  {
    bool at_least_one_face_on_boundary = false;

    for (unsigned short int s = 0; s < elem->n_faces(); ++s)
    {
      // get the boundary IDs that this element face lie on
      std::vector<boundary_id_type> b;
      boundary_info.boundary_ids(elem, s, b);

      // if there is no moving boundary, or no faces of this element are on any boundaries, we can leave
      if (!_has_moving_boundary || b.size() == 0)
        continue;

      // is this face on any of the specified boundaries?
      std::vector<int> indices;

      for (const auto & b_index : b)
      {
        auto it = std::find(_moving_boundary.begin(), _moving_boundary.end(), b_index);

        if (it != _moving_boundary.end())
          indices.push_back(it - _moving_boundary.begin());
      }

      // if none of this element's faces are on the boundaries of interest, we can leave
      if (indices.size() == 0)
        continue;

      if (indices.size() > 1)
        mooseError("This mesh generator does not support elements with the same face "
          "existing on multiple moving side sets!");

      unsigned int index = indices[0];

      // use the element centroid for finding the closest origin
      const Point centroid = elem->vertex_average();

      if (at_least_one_face_on_boundary)
        mooseError("This mesh generator cannot be applied to elements that have more than "
          "one face on the circular sideset!");

      at_least_one_face_on_boundary = true;
      moveElem(elem, index, s, polygon_layer_smoothing);
    }
  }

  // [4] store information about the nodes
  for (const auto & node : mesh->node_ptr_range())
  {
    original_nodes.push_back(*node);
    original_node_ids.push_back(node->id());
  }

  mesh->clear();

  // create the nodes
  for (unsigned int i = 0; i < original_nodes.size(); ++i)
  {
    Point pt(original_nodes[i](0), original_nodes[i](1), original_nodes[i](2));
    mesh->add_point(pt, original_node_ids[i]);
  }

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

  // create the sidesets
  for (unsigned int i = 0; i < boundary_elem_ids.size(); ++i)
  {
    auto elem_id = boundary_elem_ids[i];
    auto boundary = boundary_ids[i];

    const auto & elem = mesh->elem_ptr(elem_id);

    // if boundary is not included in the list to rebuild, skip it
    for (const auto & b : boundary)
    {
      if (_boundaries_to_rebuild.find(b) == _boundaries_to_rebuild.end())
        continue;
      else
        boundary_info.add_side(elem, boundary_face_ids[i], b);
    }
  }

  for (const auto & b: _boundary_id_to_name)
  {
    // if boundary is not included in the list to rebuild, skip it
    if (_boundaries_to_rebuild.find(b.first) == _boundaries_to_rebuild.end())
      continue;

    boundary_info.sideset_name(b.first) = b.second;
  }

  mesh->prepare_for_use();

  return dynamic_pointer_cast<MeshBase>(mesh);
}
