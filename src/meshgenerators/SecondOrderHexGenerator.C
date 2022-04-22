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
#include "MooseMeshUtils.h"

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
  params.addParam<std::vector<BoundaryName>>("boundary", "Boundary(s) to enforce a circular surface");
  params.addParam<std::vector<Real>>("radius", "Radius(es) of the circular surfaces");
  params.addParam<std::vector<std::vector<Real>>>("origin", "Origin(s) about which to form the circular surfaces; "
    "if not specified, all values default to (0, 0, 0)");
  params.addParam<std::vector<unsigned int>>("num_layers", "Number of layers to sweep for each "
    "boundary when forming the circular surfaces; if not specified, all values default to 0");

  // TODO: stop-gap solution until the MOOSE reactor module does a better job
  // of clearing out lingering sidesets used for stitching, but not for actual BCs/physics
  params.addParam<std::vector<BoundaryName>>("boundaries_to_rebuild",
    "Boundary(s) to retain from the original mesh in the new mesh; if not "
    "specified, all original boundaries are kept.");

  params.addClassDescription(
      "Converts a HEX27 mesh to a HEX20 mesh, while optionally preserving "
      "circular edges (which were faceted) in the HEX27 mesh.");
  return params;
}

SecondOrderHexGenerator::SecondOrderHexGenerator(const InputParameters & params)
  : MeshGenerator(params),
    _input(getMesh("input")),
    _axis(getParam<MooseEnum>("axis")),
    _has_moving_boundary(isParamValid("boundary"))
{
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

  if (isParamValid("boundary"))
    checkRequiredParam(params, "radius", "When specifying a 'boundary'");

  if (isParamValid("radius"))
    checkRequiredParam(params, "boundary", "When specifying a 'radius'");

  if (!isParamValid("boundary") && !isParamValid("radius"))
  {
    checkUnusedParam(params, "axis", "If not setting a 'boundary'");
    checkUnusedParam(params, "origin", "If not setting a 'boundary'");
    checkUnusedParam(params, "num_layers", "If not setting a 'boundary'");
  }

  _across_pair.resize(Hex27::num_sides);
  _across_pair[0] = {{0, 4}, {8, 16}, {1, 5}, {11, 19}, {20, 25}, {9, 17}, {3, 7}, {10, 18}, {2, 6}};
  _across_pair[1] = {{0, 3}, {8, 10}, {1, 2}, {12, 15}, {21, 23}, {13, 14}, {4, 7}, {16, 18}, {5, 6}};
  _across_pair[2] = {{1, 0}, {9, 11}, {2, 3}, {13, 12}, {22, 24}, {14, 15}, {5, 4}, {17, 19}, {6, 7}};
  _across_pair[3] = {{3, 0}, {10, 8}, {2, 1}, {15, 12}, {23, 21}, {14, 13}, {7, 4}, {18, 16}, {6, 5}};
  _across_pair[4] = {{0, 1}, {11, 9}, {3, 2}, {12, 13}, {24, 22}, {15, 14}, {4, 5}, {19, 17}, {7, 6}};
  _across_pair[5] = {{4, 0}, {16, 8}, {5, 1}, {19, 11}, {25, 20}, {17, 9}, {7, 3}, {18, 10}, {6, 2}};
}

Point
SecondOrderHexGenerator::projectPoint(const Point & origin, const Point & pt) const
{
  Point vec = pt - origin;
  vec(_axis) = 0.0;
  return vec;
}

Point
SecondOrderHexGenerator::adjustPointToCircle(const unsigned int & node_id, Elem * elem, const Real & radius, const Point & origin) const
{
  auto & node = elem->node_ref(node_id);
  const Point pt(node(0), node(1), node(2));

  // project point onto the circle plane and convert to unit vector
  Point xy_plane = projectPoint(origin, pt);

  Point adjustment = xy_plane.unit() * (radius - xy_plane.norm());

  node = pt + adjustment;
  return adjustment;
}

std::pair<unsigned int, unsigned int>
SecondOrderHexGenerator::pairedNodesAboutMidPoint(const unsigned int & node_id) const
{
  int index = node_id - Hex8::num_nodes;
  unsigned int p0 = Hex27::edge_nodes_map[index][0];
  unsigned int p1 = Hex27::edge_nodes_map[index][1];
  return {p0, p1};
}

void
SecondOrderHexGenerator::adjustMidPointNode(const unsigned int & node_id, Elem * elem) const
{
  std::pair<unsigned int, unsigned int> p = pairedNodesAboutMidPoint(node_id);

  auto & adjust = elem->node_ref(node_id);
  const auto & primary = elem->node_ref(p.first);
  const auto & secondary = elem->node_ref(p.second);

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

BoundaryID
SecondOrderHexGenerator::getBoundaryID(const BoundaryName & name, const MeshBase & mesh) const
{
  auto & boundary_info = mesh.get_boundary_info();
  auto id = MooseMeshUtils::getBoundaryID(name, mesh);

  const auto & all_boundaries = boundary_info.get_boundary_ids();
  if (all_boundaries.find(id) == all_boundaries.end())
    mooseError("Boundary '", name, "' was not found in the mesh!");

  return id;
}

Point
SecondOrderHexGenerator::getClosestOrigin(const unsigned int & index, const Point & pt) const
{
  const auto & candidates = _origin[index];
  double distance = std::numeric_limits<double>::max();
  int origin_index;

  int n_origins = candidates.size() / 3;
  for (int i = 0; i < n_origins; ++i)
  {
    Point origin(candidates[3 * i], candidates[3 * i + 1], candidates[3 * i + 2]);

    // get the distance to this origin
    Point d = pt - origin;
    d(_axis) = 0.0;
    Real current_distance = d.norm();

    if (current_distance < distance)
    {
      distance = current_distance;
      origin_index = i;
    }
  }

  Point closest(candidates[3 * origin_index], candidates[3 * origin_index + 1], candidates[3 * origin_index + 2]);
  return closest;
}

std::unique_ptr<MeshBase>
SecondOrderHexGenerator::generate()
{
  std::unique_ptr<MeshBase> mesh = std::move(_input);
  auto & boundary_info = mesh->get_boundary_info();

  // TODO: no real reason for this restriction, just didn't need it in the first pass
  if (!mesh->is_replicated())
    mooseError("This mesh generator does not yet support distributed mesh implementations!");

  // get the boundary movement information, and check for valid user specifications
  if (isParamValid("boundary"))
  {
    _radius = getParam<std::vector<Real>>("radius");

    for (const auto & r : _radius)
      if (r <= 0.0)
        mooseError("All entries in 'radius' must be non-zero and positive!");

    const auto & moving_names = getParam<std::vector<BoundaryName>>("boundary");
    for (const auto & name : moving_names)
      _moving_boundary.push_back(getBoundaryID(name, *mesh));

    if (_moving_boundary.size() != _radius.size())
      mooseError("'boundary' and 'radius' must be the same length!");

    if (isParamValid("origin"))
    {
      _origin = getParam<std::vector<std::vector<Real>>>("origin");

      if (_moving_boundary.size() != _origin.size())
        mooseError("'boundary' and 'origin' must be the same length!");

      // in the case of multiple origins for one boundary, check that each has correct length
      for (const auto & o : _origin)
      {
        if (o.size() == 0)
          mooseError("Zero-length entry in 'origin' detected! Please be sure that each "
            "entry in 'origin' has a length\ndivisible by 3 to represent (x, y, z) coordinates.");

        if (o.size() % 3 != 0)
          mooseError("When using multiple origins for one boundary, each entry in 'origin' "
            "must have a length\ndivisible by 3 to represent (x, y, z) coordinates!");
      }
    }
    else
    {
      // set to the default value of (0, 0, 0)
      for (const auto & r : _radius)
        _origin.push_back({0.0, 0.0, 0.0});
    }

    if (isParamValid("num_layers"))
    {
      _num_layers = getParam<std::vector<unsigned int>>("num_layers");

      if (_moving_boundary.size() != _num_layers.size())
        mooseError("'boundary' and 'num_layers' must be the same length!");
    }
    else
    {
      // set to the default values of 0
      for (const auto & b : _moving_boundary)
        _num_layers.push_back(0.0);
    }
  }

  // get information on which boundaries to rebuild, and check for valid user specifications
  if (isParamValid("boundaries_to_rebuild"))
  {
    const auto & rebuild_names = getParam<std::vector<BoundaryName>>("boundaries_to_rebuild");
    for (const auto & name : rebuild_names)
      _boundaries_to_rebuild.insert(getBoundaryID(name, *mesh));
  }
  else
  {
    // by default, rebuild all the boundaries
    const auto & all_boundaries = boundary_info.get_boundary_ids();
    for (const auto & b : all_boundaries)
      _boundaries_to_rebuild.insert(b);
  }

  // check for valid element type
  for (const auto & elem : mesh->element_ptr_range())
  {
    libMesh::Hex27 * hex27 = dynamic_cast<libMesh::Hex27 *>(elem);
    if (!hex27)
      mooseError("This mesh generator can only be applied to HEX27 elements!");
  }

  // store the existing boundary IDs and names
  for (const auto & b: boundary_info.get_boundary_ids())
    _boundary_id_to_name[b] = boundary_info.get_sideset_name(b);

  std::vector<libMesh::dof_id_type> boundary_elem_ids;
  std::vector<unsigned int> boundary_face_ids;
  std::vector<std::vector<boundary_id_type>> boundary_ids;

  // move the nodes on the surface of interest; while looping through the elements,
  // also store the boundar information
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

      if (indices.size() > 1)
        mooseError("This mesh generator does not support elements with the same face "
          "existing on multiple moving side sets!");

      // use the element centroid for finding the closest origin
      const Point centroid = elem->vertex_average();

      for (unsigned int ii = 0; ii < indices.size(); ++ii)
      {
        unsigned int index = indices[ii];

        if (at_least_one_face_on_boundary)
          mooseError("This mesh generator cannot be applied to elements that have more than "
            "one face on the circular sideset!");

        at_least_one_face_on_boundary = true;
        Point pt = getClosestOrigin(index, centroid);

        auto paired_nodes_for_face = _across_pair[s];
        for (auto & face_node : nodesOnFace(s))
        {
          Point adjustment = adjustPointToCircle(face_node, elem, _radius[index], pt);

          // move boundary layers of paired nodes, if present
          if (_num_layers[index] > 0)
          {
            // find the paired node
            unsigned int pair;
            for (const auto & p : paired_nodes_for_face)
            {
              if (p.first == face_node)
              {
                pair = p.second;
                break;
              }
            }

            for (unsigned int l = 0; l < _num_layers[index]; ++l)
            {
              auto & paired_node = elem->node_ref(pair);
              paired_node += adjustment;
            }
          }

          // if this is a corner node, we also need to adjust the mid-point node
          if (isCornerNode(face_node))
            adjustMidPointNode(midPointNodeIndex(s, face_node), elem);
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
        boundary_info.add_side(elem, boundary_face_ids[i], boundary);
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
