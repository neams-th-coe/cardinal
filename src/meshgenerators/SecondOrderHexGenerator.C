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

#include "SecondOrderHexGenerator.h"
#include "CastUniquePointer.h"

#include "libmesh/cell_hex20.h"
#include "libmesh/cell_hex27.h"

registerMooseObject("CardinalApp", SecondOrderHexGenerator);

InputParameters
SecondOrderHexGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();

  params.addRequiredParam<MeshGeneratorName>("input", "The mesh we want to modify");
  params.addRequiredParam<std::vector<BoundaryID>>("boundary", "Boundary to enforce a circular surface");
  params.addRequiredParam<Real>("radius", "Radius of circular surface");

  params.addClassDescription(
      "Converts a HEX27 mesh to a HEX20 mesh, while optionally preserving "
      "circular edges (which were faceted) in the HEX27 mesh.");
  return params;
}

SecondOrderHexGenerator::SecondOrderHexGenerator(const InputParameters & params)
  : MeshGenerator(params),
    _input(getMesh("input")),
    _boundary(getParam<std::vector<BoundaryID>>("boundary")),
    _radius(getParam<Real>("radius"))
{
}

std::unique_ptr<MeshBase>
SecondOrderHexGenerator::generate()
{
  int n_nodes_per_elem = 20;

  std::unique_ptr<MeshBase> mesh = std::move(_input);

  // loop over the mesh and store all the element information
  int N = mesh->n_elem();
  std::vector<std::vector<libMesh::dof_id_type>> node_ids;
  std::vector<libMesh::dof_id_type> elem_ids;
  std::vector<std::vector<boundary_id_type>> elem_face_ids;
  node_ids.resize(N);
  elem_face_ids.resize(N);

  const auto & b_info = mesh->get_boundary_info();

  // for each face (0 through 5), what are the node ids on that face
  std::vector<std::vector<unsigned int>> face_to_node_ids;
  face_to_node_ids.push_back({4, 16, 5, 19, 25, 17, 7, 18, 6});
  face_to_node_ids.push_back({0, 8, 1, 12, 21, 13, 4, 16, 5});
  face_to_node_ids.push_back({1, 9, 2, 13, 22, 14, 5, 17, 6});
  face_to_node_ids.push_back({3, 10, 2, 15, 23, 14, 7, 18, 6});
  face_to_node_ids.push_back({0, 11, 3, 12, 24, 15, 4, 19, 7});
  face_to_node_ids.push_back({0, 8, 1, 11, 20, 9, 3, 10, 2});

  // for each face, the mid-side nodes to be adjusted
  std::vector<std::vector<unsigned int>> side_ids;
  side_ids.push_back({12, 15, 14, 13});
  side_ids.push_back({19, 17, 9, 11});
  side_ids.push_back({16, 18, 10, 8});
  side_ids.push_back({19, 17, 9, 11});  // same as side 1
  side_ids.push_back({8, 16, 18, 10});  // same as side 2
  side_ids.push_back({12, 13, 14, 15}); // same as side 0

  // nodes to be averaged to get the mid-side points
  std::vector<std::vector<unsigned int>> paired_nodes0;
  std::vector<std::vector<unsigned int>> paired_nodes1;
  paired_nodes0.push_back({4, 7, 6, 5});
  paired_nodes0.push_back({4, 5, 1, 0});
  paired_nodes0.push_back({5, 6, 2, 1});
  paired_nodes0.push_back({7, 6, 2, 3});
  paired_nodes0.push_back({0, 4, 7, 3});
  paired_nodes0.push_back({0, 1, 2, 3});

  paired_nodes1.push_back({0, 3, 2, 1});
  paired_nodes1.push_back({7, 6, 2, 3});
  paired_nodes1.push_back({4, 7, 3, 0});
  paired_nodes1.push_back({4, 5, 1, 0});
  paired_nodes1.push_back({1, 5, 6, 2});
  paired_nodes1.push_back({4, 5, 6, 7});

  // move the nodes on the surface of interest
  for (auto & elem : mesh->element_ptr_range())
  {
    for (unsigned short int s = 0; s < 6; ++s)
    {
      std::vector<boundary_id_type> b;
      b_info.boundary_ids(elem, s, b);

      std::vector<SubdomainID> v(b.size() + _boundary.size());
      std::vector<SubdomainID>::iterator it, st;

      it = std::set_intersection(b.begin(), b.end(), _boundary.begin(), _boundary.end(), v.begin());

      for (st = v.begin(); st != it; ++st)
      {
        // this face of the element is on the boundary of interest
        const auto & face_nodes = face_to_node_ids[s];

        // nodes on the face that will need to be used to adjust the mid-side points
        const auto & primary_nodes = paired_nodes0[s];
        const auto & secondary_nodes = paired_nodes1[s];

        for (unsigned int n = 0; n < face_nodes.size(); ++n)
        {
          auto face_node = face_nodes[n];

          auto & node = elem->node_ref(face_node);
          const Point original_pt(node(0), node(1), node(2));

          // distance from origin, in the x-y plane
          const Point xy_plane(node(0), node(1), 0.0);
          Real d0 = xy_plane.norm();

          // unit vector from origin to the node
          const Point project(0.0, 0.0, node(2));
          Point unit_vector = original_pt - project;
          unit_vector = unit_vector.unit();

          const Point new_pt = original_pt + unit_vector * (_radius - d0);

          node(0) = new_pt(0);
          node(1) = new_pt(1);
          node(2) = new_pt(2);

          // if this is a corner node, we also need to adjust the mid-point node
          if (std::count(primary_nodes.begin(), primary_nodes.end(), face_node))
          {
            auto it = std::find(primary_nodes.begin(), primary_nodes.end(), face_node);
            int index = it - primary_nodes.begin();

            auto & primary = elem->node_ref(primary_nodes[index]);
            auto & secondary = elem->node_ref(secondary_nodes[index]);
            auto & adjust = elem->node_ref(side_ids[s][index]);

            Point pt(0.5 * (primary(0) + secondary(0)), 0.5 * (primary(1) + secondary(1)),
              0.5 * (primary(2) + secondary(2)));
            adjust(0) = pt(0);
            adjust(1) = pt(1);
            adjust(2) = pt(2);
          }
        }
      }
    }
  }

  int i = 0;
  for (const auto & elem : mesh->element_ptr_range())
  {
    libMesh::Hex27 * hex27 = dynamic_cast<libMesh::Hex27 *>(elem);
    if (!hex27)
      mooseError("This mesh generator can only be applied to HEX27 elements!");

    elem_ids.push_back(hex27->set_id());

    for (unsigned int j = 0; j < n_nodes_per_elem; ++j)
      node_ids[i].push_back(hex27->node_ref(j).id());

    i++;
  }

  // loop over the mesh and store all the node information
  std::vector<Node> original_nodes;
  std::vector<dof_id_type> original_node_ids;
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
    mesh->add_elem(elem);

    // node IDs for the i-th element
    const auto & ids = node_ids[i];

    for (unsigned int n = 0; n < ids.size(); ++n)
    {
      auto node_ptr = mesh->node_ptr(ids[n]);
      elem->set_node(n) = node_ptr;
    }
  }

  std::cout << "added all elemens" << std::endl;
  mesh->prepare_for_use();

  return dynamic_pointer_cast<MeshBase>(mesh);
}
