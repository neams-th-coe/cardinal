#include "HexagonalSubchannelMesh.h"
#include "libmesh/cell_prism6.h"
#include "libmesh/unstructured_mesh.h"

const Real HexagonalSubchannelMesh::COS30 = std::sqrt(3.0) / 2.0;
const Real HexagonalSubchannelMesh::SIN30 = 0.5;
const unsigned int HexagonalSubchannelMesh::NODES_PER_PRISM = 6;
const unsigned int HexagonalSubchannelMesh::NUM_SIDES = 6;

registerMooseObject("CardinalApp", HexagonalSubchannelMesh);

defineLegacyParams(HexagonalSubchannelMesh);

InputParameters
HexagonalSubchannelMesh::validParams()
{
  InputParameters params = MooseMesh::validParams();
  params.addRequiredRangeCheckedParam<Real>("bundle_pitch", "bundle_pitch > 0",
    "Bundle pitch, or flat-to-flat distance across bundle");
  params.addRequiredRangeCheckedParam<Real>("pin_pitch", "pin_pitch > 0",
    "Pin pitch, or distance between pin centers");
  params.addRequiredRangeCheckedParam<Real>("pin_diameter", "pin_diameter > 0",
    "Pin outer diameter");
  params.addRequiredRangeCheckedParam<unsigned int>("n_rings", "n_rings >= 1",
    "Number of pin rings, including the centermost pin as a 'ring'");
  params.addRangeCheckedParam<unsigned int>("axis", 2,
    "axis >= 0 & axis < 3",
    "vertical axis of the reactor (x = 0, y = 1, z = 2) along which pins are aligned");
  params.addRequiredRangeCheckedParam<unsigned int>("n_axial", "n_axial > 0",
    "Number of axial cells");
  params.addRequiredRangeCheckedParam<Real>("height", "height > 0", "Height of assembly");

  params.addRangeCheckedParam<unsigned int>("theta_res", 6, "theta_res >= 2", "Number of nodes on each pin's arc length with a channel");
  params.addRangeCheckedParam<unsigned int>("gap_res", 2, "gap_res >= 2", "Number of nodes on each gap");

  params.addParam<SubdomainID>("interior_id", 1, "Block ID to set for the interior channels");
  params.addParam<SubdomainID>("edge_id", 2, "Block ID to set for the edge channels");
  params.addParam<SubdomainID>("corner_id", 3, "Block ID to set for the corner channels");

  params.addClassDescription("Mesh respecting subchannel boundaries for a triangular lattice");
  return params;
}

HexagonalSubchannelMesh::HexagonalSubchannelMesh(const InputParameters & parameters)
  : MooseMesh(parameters),
    _bundle_pitch(getParam<Real>("bundle_pitch")),
    _pin_pitch(getParam<Real>("pin_pitch")),
    _pin_diameter(getParam<Real>("pin_diameter")),
    _n_rings(getParam<unsigned int>("n_rings")),
    _axis(getParam<unsigned int>("axis")),
    _theta_res(getParam<unsigned int>("theta_res")),
    _gap_res(getParam<unsigned int>("gap_res")),
    _n_axial(getParam<unsigned int>("n_axial")),
    _height(getParam<Real>("height")),
    _interior_id(getParam<SubdomainID>("interior_id")),
    _edge_id(getParam<SubdomainID>("edge_id")),
    _corner_id(getParam<SubdomainID>("corner_id")),
    _hex_lattice(HexagonalLatticeUtility(_bundle_pitch, _pin_pitch, _pin_diameter,
      0.0 /* wire diameter not needed for subchannel mesh, use dummy value */,
      1.0 /* wire pitch not needed for subchannel mesh, use dummy value */, _n_rings, _axis)),
    _pin_centers(_hex_lattice.pinCenters())
{
}

std::unique_ptr<MooseMesh>
HexagonalSubchannelMesh::safeClone() const
{
  return libmesh_make_unique<HexagonalSubchannelMesh>(*this);
}

void
HexagonalSubchannelMesh::buildMesh()
{
  MeshBase & mesh = getMesh();
  mesh.clear();
  mesh.set_mesh_dimension(3);
  mesh.set_spatial_dimension(3);

  _elems_per_interior = 3 * (_theta_res - 1) + 3 * (_gap_res - 1);
  _elems_per_edge = 2 * (_theta_res - 1) + 4 * (_gap_res - 1);
  _elems_per_corner = (_theta_res - 1) + 4 * (_gap_res - 1);

  _elem_id_counter = 0;
  _node_id_counter = 0;

  Real dz = _height / _n_axial;
  for (int i = 0; i < _n_axial; ++i)
  {
    Real zmin = i * dz;
    Real zmax = (i + 1) * dz;

    if (_hex_lattice.nInteriorChannels())
      addInteriorChannels(zmin, zmax);

    if (_hex_lattice.nEdgeChannels())
      addEdgeChannels(zmin, zmax);

    // there are always corner channels
    addCornerChannels(zmin, zmax);
  }

  mesh.prepare_for_use();
}

void
HexagonalSubchannelMesh::addCornerChannels(const Real & zmin, const Real & zmax)
{
  // Build an array of points corresponding to a corner channel relative to a centroid of (0, 0, 0)
  getCornerPoints();

  for (int i = 0; i < _hex_lattice.nCornerChannels(); ++i)
  {
    Point centroid = _hex_lattice.channelCentroid(_hex_lattice.cornerChannelCornerCoordinates(i));
    addCornerChannel(centroid, zmin, zmax, i * 2 * M_PI / NUM_SIDES);
  }
}

void
HexagonalSubchannelMesh::addEdgeChannels(const Real & zmin, const Real & zmax)
{
  // Build an array of points corresponding to an edge channel relative to a centroid of (0, 0, 0)
  getEdgePoints();

  Real rotation = 0.0;
  for (int i = 0; i < _hex_lattice.nEdgeChannels(); ++i)
  {
    if (i >= (_n_rings - 1) && i % (_n_rings - 1) == 0)
      rotation += 2 * M_PI / 6.0;

    Point centroid = _hex_lattice.channelCentroid(_hex_lattice.edgeChannelCornerCoordinates(i));
    addEdgeChannel(centroid, zmin, zmax, rotation);
  }
}

void
HexagonalSubchannelMesh::addInteriorChannels(const Real & zmin, const Real & zmax)
{
  // Build an array of points corresponding to an interior channel relative to a centroid of (0, 0, 0)
  getInteriorPoints();

  // Then add the elements for the interior channels
  for (int i = 0; i < _hex_lattice.nInteriorChannels(); ++i)
  {
    Point centroid = _hex_lattice.channelCentroid(_hex_lattice.interiorChannelCornerCoordinates(i));
    Real rotation = i % 2 == 0 ? M_PI : 0.0;
    addInteriorChannel(centroid, zmin, zmax, rotation);
  }
}

void
HexagonalSubchannelMesh::addPrismElem(const Point & pt1, const Point & pt2, const Point & pt3, const Real & zmin, const Real & zmax,
  const SubdomainID & id)
{
  auto elem = new Prism6;
  elem->set_id(_elem_id_counter++);
  _mesh->add_elem(elem);

  Point z0(0.0, 0.0, zmin);
  Point z1(0.0, 0.0, zmax);

  auto node_ptr0 = _mesh->add_point(pt1 + z0, _node_id_counter++);
  auto node_ptr1 = _mesh->add_point(pt2 + z0, _node_id_counter++);
  auto node_ptr2 = _mesh->add_point(pt3 + z0, _node_id_counter++);
  auto node_ptr3 = _mesh->add_point(pt1 + z1, _node_id_counter++);
  auto node_ptr4 = _mesh->add_point(pt2 + z1, _node_id_counter++);
  auto node_ptr5 = _mesh->add_point(pt3 + z1, _node_id_counter++);

  elem->set_node(0) = node_ptr0;
  elem->set_node(1) = node_ptr1;
  elem->set_node(2) = node_ptr2;
  elem->set_node(3) = node_ptr3;
  elem->set_node(4) = node_ptr4;
  elem->set_node(5) = node_ptr5;

  elem->subdomain_id() = id;
}

void
HexagonalSubchannelMesh::addCornerChannel(const Point & centroid, const Real & zmin, const Real & zmax, const Real & rotation)
{
  std::vector<Point> points;
  for (const auto & i : _corner_points)
    points.push_back(rotatePoint(i, rotation));

  for (int i = 0; i < _elems_per_corner; ++i)
  {
    bool last_elem = (i == _elems_per_corner - 1);

    Point pt1 = centroid + points[0];
    Point pt2 = centroid + points[i + 1];
    Point pt3 = last_elem ? centroid + points[1] : centroid + points[i + 2];
    addPrismElem(pt1, pt2, pt3, zmin, zmax, _corner_id);
  }
}

void
HexagonalSubchannelMesh::addEdgeChannel(const Point & centroid, const Real & zmin, const Real & zmax, const Real & rotation)
{
  std::vector<Point> points;
  for (const auto & i : _edge_points)
    points.push_back(rotatePoint(i, rotation));

  for (int i = 0; i < _elems_per_edge; ++i)
  {
    bool last_elem = (i == _elems_per_edge - 1);

    Point pt1 = centroid + points[0];
    Point pt2 = centroid + points[i + 1];
    Point pt3 = last_elem ? centroid + points[1] : centroid + points[i + 2];
    addPrismElem(pt1, pt2, pt3, zmin, zmax, _edge_id);
  }
}

void
HexagonalSubchannelMesh::addInteriorChannel(const Point & centroid, const Real & zmin, const Real & zmax, const Real & rotation)
{
  std::vector<Point> points;
  for (const auto & i : _interior_points)
    points.push_back(rotatePoint(i, rotation));

  for (int i = 0; i < _elems_per_interior; ++i)
  {
    bool last_elem = (i == _elems_per_interior - 1);

    Point pt1 = centroid + points[0];
    Point pt2 = centroid + points[i + 1];
    Point pt3 = last_elem ? centroid + points[1] : centroid + points[i + 2];
    addPrismElem(pt1, pt2, pt3, zmin, zmax, _interior_id);
  }
}

void
HexagonalSubchannelMesh::getInteriorPoints()
{
  int p = 0;
  _interior_points.resize(3 * _theta_res + 3 * (_gap_res - 2) + 1);
  _interior_points[p++] = Point(0.0, 0.0, 0.0);

  Real distance_from_centroid = _hex_lattice.triangleHeight(_pin_pitch) * 2.0 / 3.0;
  Real total_interior_pin_theta = 2.0 * M_PI / 6.0;
  Real pin_arc_theta = total_interior_pin_theta / (_theta_res - 1.0);
  Real gap_dx = (_pin_pitch - _pin_diameter) / (_gap_res - 1.0);
  Real r = _hex_lattice.pinRadius();

  // center coordinates of the three pins forming the channel corners
  Point pin1(0.0, distance_from_centroid, 0.0);
  Point pin2(-distance_from_centroid * COS30, -distance_from_centroid * SIN30, 0.0);
  Point pin3(distance_from_centroid * COS30, -distance_from_centroid * SIN30, 0.0);

  // Add the points on the first pin
  for (int i = 0; i < _theta_res; ++i)
  {
    Real phi = total_interior_pin_theta + i * pin_arc_theta;
    _interior_points[p++] = pin1 + Point(r * std::cos(phi), -r * std::sin(phi), 0.0);
  }

  // Add the points on the first gap
  Point start1 = _interior_points[_theta_res];
  for (int i = 0; i < _gap_res - 2; ++i)
    _interior_points[p++] = start1 + Point(-gap_dx * (i + 1) * SIN30, -gap_dx * (i + 1) * COS30, 0.0);

  // Add the points on the second pin
  for (int i = 0; i < _theta_res; ++i)
  {
    Real phi = total_interior_pin_theta - i * pin_arc_theta;
    _interior_points[p++] = pin2 + Point(r * std::cos(phi), r * std::sin(phi), 0.0);
  }

  // Add the points on the second gap
  Point start2 = _interior_points[2 * _theta_res + (_gap_res - 2)];
  for (int i = 0; i < _gap_res - 2; ++i)
    _interior_points[p++] = start2 + Point(gap_dx * (i + 1), 0.0, 0.0);

  // Add points on the third pin
  for (int i = 0; i < _theta_res; ++i)
  {
    Real phi = i * pin_arc_theta;
    _interior_points[p++] = pin3 + Point(-r * std::cos(phi), r * std::sin(phi), 0.0);
  }

  // Add points on the third gap
  Point start3 = _interior_points[3 * _theta_res + 2 * (_gap_res - 2)];
  for (int i = 0; i < _gap_res - 2; ++i)
    _interior_points[p++] = start3 + Point(-gap_dx * (i + 1) * SIN30, gap_dx * (i + 1) * COS30, 0.0);
}

void
HexagonalSubchannelMesh::getEdgePoints()
{
  int p = 0;
  _edge_points.resize(2 * _theta_res + 2 * (_gap_res - 2) + 2 * (_gap_res - 1) + 1);

  Real width = _pin_pitch;
  Real r = _hex_lattice.pinRadius();
  Real height = _hex_lattice.pinBundleSpacing() + r;
  Real vertical_gap_arc_length = _hex_lattice.pinBundleSpacing() / (_gap_res - 1.0);
  Real horizontal_gap_arc_length = (_pin_pitch - _pin_diameter) / (_gap_res - 1.0);
  Real total_edge_pin_theta = M_PI / 2.0;
  Real pin_arc_theta = total_edge_pin_theta / (_theta_res - 1.0);

  // center point is at half the distance from the pin outer surface to the duct
  _edge_points[p++] = Point(0.0, r + _hex_lattice.pinBundleSpacing() / 2.0 - height / 2.0, 0.0);

  // Add points on the first gap
  Point start1 = Point(-width / 2.0, height / 2.0, 0.0);
  for (int i = 0; i < _gap_res - 1; ++i)
    _edge_points[p++] = start1 + Point(0.0, -vertical_gap_arc_length * i, 0.0);

  // Add points on the first pin
  Point pin1 = Point(-width / 2.0, -height / 2.0, 0.0);
  for (int i = 0; i < _theta_res; ++i)
  {
    Real phi = pin_arc_theta * i;
    _edge_points[p++] = pin1 + Point(r * std::sin(phi), r * std::cos(phi), 0.0);
  }

  // Add points on the second gap
  Point start2 = _edge_points[p - 1];
  for (int i = 0; i < _gap_res - 2; ++i)
    _edge_points[p++] = start2 + Point((i + 1) * horizontal_gap_arc_length, 0.0, 0.0);

  // Add points on the second pin
  Point pin2 = Point(width / 2.0, -height / 2.0, 0.0);
  for (int i = 0; i < _theta_res; ++i)
  {
    Real phi = i * pin_arc_theta;
    _edge_points[p++] = pin2 + Point(-r * std::cos(phi), r * std::sin(phi), 0.0);
  }

  // Add points on the third gap
  Point start3 = _edge_points[p - 1];
  for (int i = 0; i < _gap_res - 1; ++i)
    _edge_points[p++] = start3 + Point(0.0, vertical_gap_arc_length * (i + 1), 0.0);

  // Add points on the duct
  Point start4 = _edge_points[p - 1];
  for (int i = 0; i < _gap_res - 2; ++i)
    _edge_points[p++] = start4 + Point(-(i + 1) * _pin_pitch / (_gap_res - 1.0), 0.0, 0.0);
}

void
HexagonalSubchannelMesh::getCornerPoints()
{
  int p = 0;
  _corner_points.resize(_theta_res + 4 * (_gap_res - 1) + 1);

  // Start with the first corner channel, then we'll translate the centroid (relative
  // to the bundle center) to (0, 0, 0)
  std::vector<Point> pts = _hex_lattice.cornerChannelCornerCoordinates(0);

  Point centroid = _hex_lattice.channelCentroid(pts);
  for (auto & pt : pts)
    pt -= centroid;

  // the first point is at the intersection between two lines at the
  // halfway point between the pin surface and the duct
  Real r = _hex_lattice.pinRadius();
  Real dy = pts[0](1) + r + _hex_lattice.pinBundleSpacing() / 2.0;
  _corner_points[p++] = Point(dy * SIN30 / COS30, dy, 0.0);

  Real gap_arc_length = _hex_lattice.pinBundleSpacing() / (_gap_res - 1.0);
  Real total_corner_pin_theta = 2.0 * M_PI / 6.0;
  Real pin_arc_theta = total_corner_pin_theta / (_theta_res - 1.0);

  // Add the points on the first gap
  for (int i = 0; i < _gap_res - 1; ++i)
    _corner_points[p++] = pts[3] + Point(0.0, -i * gap_arc_length, 0.0);

  // Add the points on the pin
  for (int i = 0; i < _theta_res; ++i)
  {
    Real phi = i * pin_arc_theta;
    _corner_points[p++] = pts[0] + Point(r * std::sin(phi), r * std::cos(phi), 0.0);
  }

  // Add the points on the second gap
  for (int i = 0; i < _gap_res - 1; ++i)
  {
    Real dx = (i + 1) * gap_arc_length;
    _corner_points[p++] = _corner_points[_theta_res + _gap_res - 1] + Point(dx * COS30, dx * SIN30, 0.0);
  }

  // Add the points on the duct
  Point wall = pts[2] - pts[1];
  for (int i = 0; i < _gap_res - 1; ++i)
    _corner_points[p++] = pts[1] + wall * (i + 1) / (_gap_res - 1.0);

  wall = pts[3] - pts[2];
  for (int i = 0; i < _gap_res - 2; ++i)
    _corner_points[p++] = pts[2] + wall * (i + 1) / (_gap_res - 1.0);
}

const Point
HexagonalSubchannelMesh::rotatePoint(const Point & p, const Real & theta) const
{
  Real x = p(0);
  Real y = p(1);
  Point rotation(x * std::cos(theta) - y * std::sin(theta), x * std::sin(theta) +y * std::cos(theta), 0.0);
  return rotation;
}
