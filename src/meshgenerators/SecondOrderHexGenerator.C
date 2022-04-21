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
#include "UserErrorChecking.h"

#include "libmesh/mesh_tools.h"
#include "libmesh/cell_hex20.h"
#include "libmesh/cell_hex8.h"

registerMooseObject("CardinalApp", SecondOrderHexGenerator);

InputParameters
SecondOrderHexGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();
  params.addRequiredParam<MeshGeneratorName>("input", "The mesh we want to modify");

  // optional parameters if fitting sidesets to a circular surface
  MooseEnum axis("x y z", "z");
  params.addParam<MooseEnum>("axis", axis, "Axis of the mesh about which to build "
    "the circular surface");
  params.addParam<std::vector<BoundaryID>>("boundary", "Boundary(s) to enforce a circular surface");
  params.addParam<Real>("radius", "Radius the circular surface");

  params.addClassDescription(
      "Converts a HEX27 mesh to a HEX20 mesh, while optionally preserving "
      "circular edges (which were faceted) in the HEX27 mesh.");
  return params;
}

SecondOrderHexGenerator::SecondOrderHexGenerator(const InputParameters & params)
  : MeshGenerator(params),
    _input(getMesh("input")),
    _axis(getParam<MooseEnum>("axis"))
{
  if (isParamValid("boundary"))
  {
    _boundary = &getParam<std::vector<BoundaryID>>("boundary");
    checkRequiredParam(params, "radius", "When specifying a 'boundary'");
  }

  if (isParamValid("radius"))
  {
    _radius = getParam<Real>("radius");
    checkRequiredParam(params, "boundary", "When specifying a 'radius'");
  }

  if (!_boundary)
    checkUnusedParam(params, "axis", "If not setting a 'boundary'");

  // for each face, the mid-side nodes to be adjusted
  _side_ids.push_back({12, 15, 14, 13});
  _side_ids.push_back({11, 9, 17, 19});
  _side_ids.push_back({8, 18, 10, 16});
  _side_ids.push_back({9, 11, 19, 17});
  _side_ids.push_back({10, 8, 16, 18});
  _side_ids.push_back({12, 13, 14, 15});
}

void
SecondOrderHexGenerator::adjustPointToCircle(const unsigned int & node_id, Elem * elem) const
{
  auto & node = elem->node_ref(node_id);

  // point to node in (x, y, z) space
  const Point pt(node(0), node(1), node(2));

  // project point onto the circle plane and convert to unit vector
  Point xy_plane = pt;
  xy_plane(_axis) = 0.0;
  Real d0 = xy_plane.norm();
  xy_plane = xy_plane.unit();

  node = pt + xy_plane * (_radius - d0);
}

void
SecondOrderHexGenerator::adjustMidPointNode(const unsigned int & node_id, Elem * elem) const
{
  int index = node_id - Hex8::num_nodes;
  unsigned int p0 = Hex27::edge_nodes_map[index][0];
  unsigned int p1 = Hex27::edge_nodes_map[index][1];

  auto & adjust = elem->node_ref(node_id);
  const auto & primary = elem->node_ref(p0);
  const auto & secondary = elem->node_ref(p1);

  Point pt(0.5 * (primary(0) + secondary(0)), 0.5 * (primary(1) + secondary(1)),
           0.5 * (primary(2) + secondary(2)));

  adjust = pt;
}

unsigned int
SecondOrderHexGenerator::midPointNodeIndex(const unsigned int & face_id, const unsigned int & face_node) const
{
  const auto & primary_nodes = _corner_nodes[face_id];
  auto it = std::find(primary_nodes.begin(), primary_nodes.end(), face_node);
  return _side_ids[face_id][it - primary_nodes.begin()];
}

std::unique_ptr<MeshBase>
SecondOrderHexGenerator::generate()
{
  std::unique_ptr<MeshBase> mesh = std::move(_input);

  // first, check for valid element type
  for (const auto & elem : mesh->element_ptr_range())
  {
    libMesh::Hex27 * hex27 = dynamic_cast<libMesh::Hex27 *>(elem);
    if (!hex27)
      mooseError("This mesh generator can only be applied to HEX27 elements!");
  }

  // next, check that the mesh is centered on the origin (for the plane of the circle)
  const auto bbox = MeshTools::create_bounding_box(*mesh);
  auto origin = 0.5 * (bbox.max() + bbox.min());

  auto adjusted_origin = origin;
  adjusted_origin(_axis) = 0.0;

  Point zero(0.0, 0.0, 0.0);
  auto desired_origin = zero;
  desired_origin(_axis) = origin(_axis);

  if (!adjusted_origin.absolute_fuzzy_equals(zero))
    mooseError("This mesh generator can only be applied to meshes centered on the origin! "
      "Your origin is at: (", origin(0), ", ", origin(1), ", ", origin(2), ").\n"
      "Please use a TransformGenerator to center the input mesh at (",
       desired_origin(0), ", ", desired_origin(1), ", ", desired_origin(2), ").");

  const auto & boundary_info = mesh->get_boundary_info();

  // corner nodes for each face
  _corner_nodes.resize(Hex27::num_sides);
  for (unsigned int i = 0; i < Hex27::num_sides; ++i)
    for (unsigned int j = 0; j < Hex8::nodes_per_side; ++j)
      _corner_nodes[i].push_back(Hex27::side_nodes_map[i][j]);

  std::vector<libMesh::dof_id_type> boundary_elem_ids;
  std::vector<unsigned int> boundary_face_ids;
  std::vector<std::vector<boundary_id_type>> boundary_ids;

  // move the nodes on the surface of interest
  if (_boundary)
  {
    for (auto & elem : mesh->element_ptr_range())
    {
      bool at_least_one_face_on_boundary = false;

      for (unsigned short int s = 0; s < elem->n_faces(); ++s)
      {
        // get the boundary IDs that this element face lie on
        std::vector<boundary_id_type> b;
        boundary_info.boundary_ids(elem, s, b);

        boundary_elem_ids.push_back(elem->set_id());
        boundary_face_ids.push_back(s);
        boundary_ids.push_back(b);

        // find the overlap with the specified _boundary
        std::vector<SubdomainID> v(b.size() + _boundary->size());
        std::vector<SubdomainID>::iterator it, st;
        it = std::set_intersection(b.begin(), b.end(), _boundary->begin(), _boundary->end(), v.begin());

        for (st = v.begin(); st != it; ++st)
        {
          if (at_least_one_face_on_boundary)
            mooseError("This mesh generator cannot be applied to elements that have more than "
              "one face on the circular sideset!");

          at_least_one_face_on_boundary = true;

          for (auto & face_node : nodesOnFace(s))
          {
            adjustPointToCircle(face_node, elem);

            // if this is a corner node, we also need to adjust the mid-point node
            if (isCornerNode(face_node))
              adjustMidPointNode(midPointNodeIndex(s, face_node), elem);
          }
        }
      }
    }
  }

  // loop over the mesh and store all the element information
  int N = mesh->n_elem();
  std::vector<std::vector<libMesh::dof_id_type>> node_ids;
  node_ids.resize(N);

  std::vector<libMesh::dof_id_type> elem_ids;
  std::vector<libMesh::subdomain_id_type> elem_block_ids;

  int i = 0;
  for (const auto & elem : mesh->element_ptr_range())
  {
    libMesh::Hex27 * hex27 = dynamic_cast<libMesh::Hex27 *>(elem);
    elem_ids.push_back(hex27->set_id());
    elem_block_ids.push_back(hex27->subdomain_id());

    for (unsigned int j = 0; j < Hex20::num_nodes; ++j)
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
  auto & new_boundary_info = mesh->get_boundary_info();
  for (unsigned int i = 0; i < boundary_elem_ids.size(); ++i)
  {
    auto elem_id = boundary_elem_ids[i];
    auto face_id = boundary_face_ids[i];
    auto boundary = boundary_ids[i];

    const auto & elem = mesh->elem_ptr(elem_id);
    new_boundary_info.add_side(elem, face_id, boundary);
  }

  mesh->prepare_for_use();

  return dynamic_pointer_cast<MeshBase>(mesh);
}
