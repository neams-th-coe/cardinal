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

#include "NekMeshGenerator.h"
#include "CastUniquePointer.h"
#include "UserErrorChecking.h"
#include "MooseMeshUtils.h"
#include "DelimitedFileReader.h"
#include "GeometryUtility.h"

#include "libmesh/mesh_tools.h"

InputParameters
NekMeshGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();
  params.addRequiredParam<MeshGeneratorName>("input", "The mesh we want to modify");

  // optional parameters if fitting sidesets to a circular surface
  MooseEnum axis("x y z", "z");
  params.addParam<MooseEnum>("axis", axis, "Axis of the mesh about which to build "
    "the circular surface(s)");
  params.addParam<std::vector<BoundaryName>>("boundary", "Boundary(s) to enforce a circular surface");
  params.addParam<std::vector<Real>>("radius", "Radius(es) of the circular surfaces");
  params.addParam<std::vector<std::vector<Real>>>("origins", "Origin(s) about which to form the circular surfaces; "
    "if not specified, all values default to (0, 0, 0)");
  params.addParam<std::vector<std::string>>("origins_files", "Origin(s) about which to form the circular surfaces, "
    "with a file of points provided for each boundary. If not specified, all values default to (0, 0, 0)");
  params.addParam<std::vector<unsigned int>>("layers", "Number of layers to sweep for each "
    "boundary when forming the circular surfaces; if not specified, all values default to 0");

  // optional parameters if fitting corners of a polygon to radius of curvature
  params.addParam<bool>("curve_corners", false,
    "Whether to move elements to respect radius of curvature of polygon corners");
  params.addRangeCheckedParam<unsigned int>("polygon_sides", "polygon_sides > 2",
    "When curving corners, the number of sides of the polygon to use for identifying corners");
  params.addRangeCheckedParam<Real>("polygon_size", "polygon_size > 0.0",
    "When curving corners, the size of the polygon (measured as distance from center to a "
    "corner) to use for identifying corners");
  params.addRangeCheckedParam<Real>("corner_radius", "corner_radius > 0.0",
    "When curving corners, the radius of curvature of the corners");
  params.addParam<unsigned int>("polygon_layers", 0,
    "When curving corners, the number of layers to sweep for each polygon corner");
  params.addParam<std::vector<Real>>("polygon_layer_smoothing",
    "When curving corners, the multiplicative factor to apply to each boundary layer; if not "
    "specified, all values default to 1.0");
  params.addParam<BoundaryName>("polygon_boundary", "Boundary to enforce radius of curvature "
    "for polygon corners");
  params.addParam<std::vector<std::vector<Real>>>("polygon_origins", "Origin(s) about which to curve "
    "the polygon corners; if not specified, defaults to (0, 0, 0)");
  params.addParam<Real>("rotation_angle", 0, "When curving corners, the rotation angle (degrees) "
    "needed to apply to the original mesh to get a polygon boundary with one side horizontal");

  // TODO: stop-gap solution until the MOOSE reactor module does a better job
  // of clearing out lingering sidesets used for stitching, but not for actual BCs/physics
  params.addParam<std::vector<BoundaryName>>("boundaries_to_rebuild",
    "Boundary(s) to retain from the original mesh in the new mesh; if not "
    "specified, all original boundaries are kept.");

  params.addClassDescription(
      "Base class for converting MOOSE meshes to NekRS compatible format, while optionally preserving "
      "circular edges (which were faceted) in the HEX27 mesh.");
  return params;
}

NekMeshGenerator::NekMeshGenerator(const InputParameters & params)
  : MeshGenerator(params),
    _input(getMesh("input")),
    _axis(getParam<MooseEnum>("axis")),
    _curve_corners(getParam<bool>("curve_corners")),
    _rotation_angle(getParam<Real>("rotation_angle")),
    _has_moving_boundary(isParamValid("boundary") || _curve_corners)
{
  if (_curve_corners)
    checkRequiredParam(params, {"polygon_sides", "polygon_size", "polygon_boundary", "corner_radius"},
                               "'curve_corners' is true");
  else
    checkUnusedParam(params, {"polygon_sides", "polygon_size", "polygon_boundary",
                              "corner_radius", "polygon_layers", "rotation_angle",
                              "polygon_layer_smoothing", "polygon_origins"},
                              "'curve_corners' is false");

  if (isParamValid("boundary"))
    checkRequiredParam(params, "radius", "specifying a 'boundary'");

  if (isParamValid("radius"))
    checkRequiredParam(params, "boundary", "specifying a 'radius'");

  if (!isParamValid("boundary") && !isParamValid("radius"))
    checkUnusedParam(params, {"axis", "origins", "origins_files", "layers"},
                             "not setting a 'boundary'");
}

Point
NekMeshGenerator::projectPoint(const Point & origin, const Point & pt) const
{
  Point vec = pt - origin;
  vec(_axis) = 0.0;
  return vec;
}

Point
NekMeshGenerator::adjustPointToCircle(const unsigned int & node_id, Elem * elem, const Real & radius, const Point & origin) const
{
  auto & node = elem->node_ref(node_id);
  const Point pt(node(0), node(1), node(2));

  // project point onto the circle plane and convert to unit vector
  Point xy_plane = projectPoint(origin, pt);

  Point adjustment = xy_plane.unit() * (radius - xy_plane.norm());

  node = pt + adjustment;
  return adjustment;
}

BoundaryID
NekMeshGenerator::getBoundaryID(const BoundaryName & name, const MeshBase & mesh) const
{
  auto & boundary_info = mesh.get_boundary_info();
  auto id = MooseMeshUtils::getBoundaryID(name, mesh);

  const auto & all_boundaries = boundary_info.get_boundary_ids();
  if (all_boundaries.find(id) == all_boundaries.end())
    mooseError("Boundary '", name, "' was not found in the mesh!");

  return id;
}

void
NekMeshGenerator::checkPointLength(const std::vector<std::vector<Real>> & points, std::string name) const
{
  for (const auto & o : points)
  {
    if (o.size() == 0)
      mooseError("Zero-length entry in '" + name + "' detected! Please be sure that each "
        "entry in '" + name + "' has a length\ndivisible by 3 to represent (x, y, z) coordinates.");

    if (o.size() % 3 != 0)
      mooseError("When using multiple origins for one boundary, each entry in '" + name + "' "
        "must have a length\ndivisible by 3 to represent (x, y, z) coordinates!");
  }
}

void
NekMeshGenerator::getCircularSidesetInfo(std::unique_ptr<MeshBase> & mesh)
{
  if (!isParamValid("boundary"))
    return;

  _radius = getParam<std::vector<Real>>("radius");

  for (const auto & r : _radius)
    if (r <= 0.0)
      mooseError("All entries in 'radius' must be non-zero and positive!");

  const auto & moving_names = getParam<std::vector<BoundaryName>>("boundary");
  for (const auto & name : moving_names)
    _moving_boundary.push_back(getBoundaryID(name, *mesh));

  if (_moving_boundary.size() != _radius.size())
    mooseError("'boundary' and 'radius' must be the same length!"
      "\n 'boundary' length: ", _moving_boundary.size(), "\n 'radius' length: ", _radius.size());

  if (isParamValid("origins") && isParamValid("origins_files"))
    mooseError("Cannot specify both 'origins' and 'origins_files'!");

  if (isParamValid("origins"))
  {
    _origin = getParam<std::vector<std::vector<Real>>>("origins");

    if (_moving_boundary.size() != _origin.size())
      mooseError("'boundary' and 'origins' must be the same length!"
        "\n 'boundary' length: ", _moving_boundary.size(), "\n 'origins' length: ", _origin.size());

    checkPointLength(_origin, "origins");
  }
  else if (isParamValid("origins_files"))
  {
    auto origin_filenames = getParam<std::vector<std::string>>("origins_files");

    if (_moving_boundary.size() != origin_filenames.size())
      mooseError("'boundary' and 'origins_files' must be the same length!"
        "\n 'boundary' length: ", _moving_boundary.size(), "\n 'origins_files' length: ",
        origin_filenames.size());

    _origin.resize(origin_filenames.size());

    int i = 0;
    for (const auto & f : origin_filenames)
    {
      MooseUtils::DelimitedFileReader file(f, &_communicator);
      file.setFormatFlag(MooseUtils::DelimitedFileReader::FormatFlag::ROWS);
      file.read();

      const std::vector<std::vector<double>> & data = file.getData();

      for (const auto & d : data)
      {
        if (d.size() != 3)
          mooseError("All entries in '", f, "' must contain exactly 3 entries to represent (x, y, z) coordinates!");

        _origin[i].push_back(d[0]);
        _origin[i].push_back(d[1]);
        _origin[i].push_back(d[2]);
      }

      i += 1;
    }
  }
  else
  {
    // set to the default value of (0, 0, 0)
    for (const auto & r : _radius)
      _origin.push_back({0.0, 0.0, 0.0});
  }

  if (isParamValid("layers"))
  {
    _layers = getParam<std::vector<unsigned int>>("layers");

    if (_moving_boundary.size() != _layers.size())
      mooseError("'boundary' and 'layers' must be the same length!"
        "\n 'boundary' length: ", _moving_boundary.size(), "\n 'layers' length: ", _layers.size());
  }
  else
  {
    // set to the default values of 0
    for (const auto & b : _moving_boundary)
      _layers.push_back(0.0);
  }
}

std::vector<Real>
NekMeshGenerator::getPolygonSmoothingInfo(std::unique_ptr<MeshBase> & mesh)
{
  std::vector<Real> polygon_layer_smoothing;
  if (_curve_corners)
  {
    auto polygon_sides = getParam<unsigned int>("polygon_sides");
    auto polygon_size = getParam<Real>("polygon_size");
    auto corner_radius = getParam<Real>("corner_radius");
    auto polygon_layers = getParam<unsigned int>("polygon_layers");

    std::vector<std::vector<Real>> polygon_origin;
    if (isParamValid("polygon_origins"))
    {
      polygon_origin = getParam<std::vector<std::vector<Real>>>("polygon_origins");
      checkPointLength(polygon_origin, "polygon_origins");

      // Cannot specify a non-zero rotation when giving a different polygon origin,
      // because we simply rotate the point about either the x, y, or z 'axis'. This
      // is not a hard limit, just something we didn't initially put effort to support.
      if (std::abs(_rotation_angle) > 1e-8)
        mooseError("Cannot specify a non-zero 'rotation_angle' when providing custom 'polygon_origins'!");
    }
    else
      polygon_origin.push_back({0.0, 0.0, 0.0});

    if (polygon_layers)
    {
      if (isParamValid("polygon_layer_smoothing"))
      {
        polygon_layer_smoothing = getParam<std::vector<Real>>("polygon_layer_smoothing");
        if (polygon_layers != polygon_layer_smoothing.size())
          mooseError("The length of 'polygon_layer_smoothing' must be equal to 'polygon_layers'!");

        for (auto & p : polygon_layer_smoothing)
          if (p <= 0.0)
            mooseError("Each entry in 'polygon_layer_smoothing' must be positive and non-zero!");
      }
      else
      {
        for (unsigned int i = 0; i < polygon_layers; ++i)
          polygon_layer_smoothing.push_back(1.0);
      }
    }
    else
      checkUnusedParam(parameters(), "polygon_layer_smoothing", "not setting 'polygon_layers'");

    Real polygon_angle = M_PI - (2.0 * M_PI / polygon_sides);
    Real max_circle_radius = polygon_size * std::cos(M_PI / polygon_sides);

    if (corner_radius > max_circle_radius)
      mooseError("Specified 'corner_radius' cannot fit within the specified polygon!\n"
        "The maximum allowable radius of curvature is: ", max_circle_radius);

    const auto & name = getParam<BoundaryName>("polygon_boundary");
    auto polygon_boundary = getBoundaryID(name, *mesh);

    // polygon boundary shouldn't already have been specified in the 'boundary'
    if (std::count(_moving_boundary.begin(), _moving_boundary.end(), polygon_boundary))
      mooseError("The 'polygon_boundary' cannot also be listed in the 'boundary'!");

    Real theta = M_PI / 2.0 - polygon_angle / 2.0;
    Real l = corner_radius / std::cos(theta);
    _max_corner_distance = l * std::sin(theta);

    // find origins of the cylinders for the corner fitting
    std::vector<Point> corner_origins;
    for (const auto & o : polygon_origin)
    {
      Point shift(o[0], o[1], o[2]);
      auto tmp1 = geom_utility::polygonCorners(polygon_sides, polygon_size, _axis);
      for (const auto & t : tmp1)
        _polygon_corners.push_back(t + shift);

      auto tmp2 = geom_utility::polygonCorners(polygon_sides, polygon_size - l, _axis);
      for (const auto & t : tmp2)
        corner_origins.push_back(t + shift);
    }
    // apply optional rotation
    Real rotation_angle_radians = _rotation_angle * M_PI / 180.0;
    Point axis(0.0, 0.0, 0.0);
    axis(_axis) = 1.0;
    for (auto & o : _polygon_corners)
      o = geom_utility::rotatePointAboutAxis(o, rotation_angle_radians, axis);
    for (auto & o : corner_origins)
      o = geom_utility::rotatePointAboutAxis(o, rotation_angle_radians, axis);

    std::vector<Real> flattened_corner_origins;
    for (const auto & o : corner_origins)
      for (unsigned int i = 0; i < 3; ++i)
        flattened_corner_origins.push_back(o(i));

    // We can treat the polygon corners simply as extra entries in the
    // boundary, origins, and radii vectors
    _n_noncorner_boundaries = _moving_boundary.size();
    _moving_boundary.push_back(polygon_boundary);
    _radius.push_back(corner_radius);
    _origin.push_back(flattened_corner_origins);
    _layers.push_back(getParam<unsigned int>("polygon_layers"));
  }

  return polygon_layer_smoothing;
}

void
NekMeshGenerator::getBoundariesToRebuild(std::unique_ptr<MeshBase> & mesh)
{
  auto & boundary_info = mesh->get_boundary_info();
  const auto & original_boundaries = boundary_info.get_boundary_ids();

  if (isParamValid("boundaries_to_rebuild"))
  {
    const auto & rebuild_names = getParam<std::vector<BoundaryName>>("boundaries_to_rebuild");
    for (const auto & name : rebuild_names)
      _boundaries_to_rebuild.insert(getBoundaryID(name, *mesh));
  }
  else
  {
    // by default, rebuild all the boundaries
    for (const auto & b : original_boundaries)
      _boundaries_to_rebuild.insert(b);
  }
}

void
NekMeshGenerator::adjustMidPointNode(const unsigned int & node_id, Elem * elem) const
{
  std::pair<unsigned int, unsigned int> p = pairedNodesAboutMidPoint(node_id);

  auto & adjust = elem->node_ref(node_id);
  const auto & primary = elem->node_ref(p.first);
  const auto & secondary = elem->node_ref(p.second);

  Point pt(0.5 * (primary(0) + secondary(0)), 0.5 * (primary(1) + secondary(1)),
           0.5 * (primary(2) + secondary(2)));

  adjust = pt;
}

bool
NekMeshGenerator::isNearCorner(const Point & pt) const
{
  // point is a moving point by the corner if the distance from the corner is less
  // that the distance from the circle tangent to the corner
  Real distance_to_closest_corner = geom_utility::minDistanceToPoints(pt,
    _polygon_corners, _axis);

  return distance_to_closest_corner < _max_corner_distance;
}

unsigned int
NekMeshGenerator::getNodeIndex(const Elem * elem, const Point & pt) const
{
  for (unsigned int i = 0; i < _n_start_nodes; ++i)
    if (pt.absolute_fuzzy_equals(elem->point(i)))
      return i;

  mooseError("Failed to find any node on element ", elem->id(), " that matches ", pt);
}

Point
NekMeshGenerator::getClosestOrigin(const unsigned int & index, const Point & pt) const
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

void
NekMeshGenerator::moveElem(Elem * elem, const unsigned int & boundary_index, const unsigned int & primary_face,
  const std::vector<Real> & polygon_layer_smoothing)
{
  bool is_corner_boundary = boundary_index >= _n_noncorner_boundaries;

  const auto bl_elems = getBoundaryLayerElems(elem, _layers[boundary_index], primary_face);

  // use the element centroid for finding the closest origin
  const Point centroid = elem->vertex_average();
  Point pt = getClosestOrigin(boundary_index, centroid);

  for (auto & face_node : nodesOnFace(primary_face))
  {
    const Node & n = elem->node_ref(face_node);
    const Point corner_point(n(0), n(1), n(2));
    if (is_corner_boundary && !isNearCorner(corner_point))
      continue;

    // move the points on the primary face
    Point adjustment = adjustPointToCircle(face_node, elem, _radius[boundary_index], pt);

    // move boundary layers of paired nodes, if present
    Elem * bl_elem = elem;
    unsigned int start_node = face_node;
    unsigned int start_face = primary_face;
    unsigned int pair_node = pairedFaceNode(start_node, start_face);

    for (unsigned int l = 0; l < _layers[boundary_index]; ++l)
    {
      auto & paired_node = bl_elem->node_ref(pair_node);
      Real multiplier = is_corner_boundary ? polygon_layer_smoothing[l] : 1.0;

      paired_node += adjustment * multiplier;

      // if this is a corner node, we also need to adjust the mid-point node
      if (isCornerNode(start_node))
        adjustMidPointNode(midPointNodeIndex(start_face, start_node), bl_elem);

      // increment to the next boundary layer element
      auto next_elem = bl_elems[l];
      bl_elem = const_cast<Elem *>(next_elem);
      Point pt(paired_node(0), paired_node(1), paired_node(2));
      start_node = getNodeIndex(bl_elem, pt);
      pair_node = pairedFaceNode(start_node, start_face);
    }

    // even if there aren't boundary layers, we need to adjust the mid-point side node
    // of the first moved element
    if (_layers[boundary_index] == 0)
      if (isCornerNode(face_node))
        adjustMidPointNode(midPointNodeIndex(primary_face, face_node), elem);
  }
}

std::vector<Elem *>
NekMeshGenerator::getBoundaryLayerElems(Elem * elem, const unsigned int & n_layers,
  const unsigned int & primary_face) const
{
  std::vector<Elem *> nested_elems;

  unsigned int face_node = getFaceNode(primary_face);

  Elem * bl_elem = elem;
  unsigned int start_face = primary_face;
  unsigned int pair_face = _across_face[start_face];

  for (unsigned int l = 0; l < n_layers; ++l)
  {
    // increment to the next boundary layer element
    auto next_elem = getNextLayerElem(*bl_elem, pair_face, start_face);
    bl_elem = const_cast<Elem *>(next_elem);
    pair_face = _across_face[start_face];
    nested_elems.push_back(bl_elem);
  }

  return nested_elems;
}

unsigned int
NekMeshGenerator::pairedFaceNode(const unsigned int & node_id, const unsigned int & face_id) const
{
  unsigned int pair;

  for (const auto & p : _across_pair[face_id])
  {
    if (p.first == node_id)
    {
      pair = p.second;
      break;
    }
  }

  return pair;
}

unsigned int
NekMeshGenerator::midPointNodeIndex(const unsigned int & face_id, const unsigned int & face_node) const
{
  const auto & primary_nodes = _corner_nodes[face_id];
  auto it = std::find(primary_nodes.begin(), primary_nodes.end(), face_node);
  return _side_ids[face_id][it - primary_nodes.begin()];
}

const Elem *
NekMeshGenerator::getNextLayerElem(const Elem & elem, const unsigned int & touching_face, unsigned int & next_touching_face) const

{
  std::set<const Elem * > neighbor_set;

  // for the input element, get the node index on the mid-face, since for Hex27 that should
  // only uniquely touch one other element
  auto face_node = getFaceNode(touching_face);
  auto face_pt = elem.point(face_node);

  elem.find_point_neighbors(face_pt, neighbor_set);

  if (neighbor_set.size() != 2)
    mooseError("Boundary layer sweeping requires finding exactly one neighbor element\n"
      "through the layer face! Found ", neighbor_set.size() - 1, " neighbors for element ",
      elem.id(), ", face ", touching_face,
      ".\n\nThis can happen if you have specified more 'layers' than are actually in your mesh.");

  // TODO: this mesh generator currently assumes we're working on an extruded mesh,
  // so that the face pattern follows as we sweep through the boundary layers
  next_touching_face = _across_face[touching_face];

  std::set<const Elem *>::iterator it = neighbor_set.begin();
  for (int i = 0; i < neighbor_set.size(); ++i, it++)
  {
    // we restrict the size to 2, so just return the element that is NOT the input element
    if ((*it)->id() != elem.id())
      return *it;
  }

  mooseError("Failed to find a neighbor element across the boundary layer! Please check that\n"
    "the 'layers' are set to reasonable values.");
}

void
NekMeshGenerator::moveNodes(std::unique_ptr<MeshBase> & mesh, std::vector<Real> & polygon_layer_smoothing)
{
  auto & boundary_info = mesh->get_boundary_info();

  // move the nodes on the surface of interest
  for (auto & elem : mesh->element_ptr_range())
  {
    bool at_least_one_face_on_boundary = false;

    for (unsigned short int s = 0; s < _n_sides; ++s)
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
}

void
NekMeshGenerator::storeMeshInfo(std::unique_ptr<MeshBase> & mesh, std::vector<dof_id_type> & boundary_elem_ids,
  std::vector<unsigned int> & boundary_face_ids, std::vector<std::vector<boundary_id_type>> & boundary_ids)
{
  auto & boundary_info = mesh->get_boundary_info();
  const auto & original_boundaries = boundary_info.get_boundary_ids();

  // store boundary ID <-> name mapping
  for (const auto & b: original_boundaries)
    _boundary_id_to_name[b] = boundary_info.get_sideset_name(b);

  int i = 0;
  for (auto & elem : mesh->element_ptr_range())
  {
    // store information about the element faces
    for (unsigned short int s = 0; s < _n_sides; ++s)
    {
      std::vector<boundary_id_type> b;
      boundary_info.boundary_ids(elem, s, b);

      boundary_elem_ids.push_back(elem->id());
      boundary_face_ids.push_back(s);
      boundary_ids.push_back(b);
    }
  }
}

void
NekMeshGenerator::addSidesets(std::unique_ptr<MeshBase> & mesh, std::vector<dof_id_type> & boundary_elem_ids,
  std::vector<unsigned int> & boundary_face_ids,
  std::vector<std::vector<boundary_id_type>> & boundary_ids)
{
  auto & boundary_info = mesh->get_boundary_info();

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
}

void
NekMeshGenerator::saveAndRebuildNodes(std::unique_ptr<MeshBase> & mesh)
{
  std::vector<Node> original_nodes;
  std::vector<dof_id_type> original_node_ids;

  // store information about the nodes
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
}

unsigned int
NekMeshGenerator::getFaceNode(const unsigned int & primary_face) const
{
  const auto face_nodes = nodesOnFace(primary_face);
  return face_nodes[_n_start_nodes_per_side - 1];
}

std::unique_ptr<MeshBase>
NekMeshGenerator::generate()
{
  mooseError("Should not get here!");
}
