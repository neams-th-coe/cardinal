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

#include "HexagonalSubchannelGapMesh.h"
#include "libmesh/face_quad4.h"
#include "libmesh/unstructured_mesh.h"

registerMooseObject("CardinalApp", HexagonalSubchannelGapMesh);

InputParameters
HexagonalSubchannelGapMesh::validParams()
{
  InputParameters params = HexagonalSubchannelMeshBase::validParams();
  params.addRequiredRangeCheckedParam<unsigned int>(
      "n_axial", "n_axial > 0", "Number of axial cells");
  params.addRequiredRangeCheckedParam<Real>("height", "height > 0", "Height of assembly");

  params.addParam<SubdomainID>("interior_id", 1, "Sideset ID to set for the interior channel gaps");
  params.addParam<SubdomainID>(
      "peripheral_id", 2, "Block ID to set for the peripheral channel gaps");

  params.addClassDescription("Mesh respecting subchannel gaps for a triangular lattice");
  return params;
}

HexagonalSubchannelGapMesh::HexagonalSubchannelGapMesh(const InputParameters & parameters)
  : HexagonalSubchannelMeshBase(parameters),
    _n_axial(getParam<unsigned int>("n_axial")),
    _height(getParam<Real>("height")),
    _interior_id(getParam<SubdomainID>("interior_id")),
    _peripheral_id(getParam<SubdomainID>("peripheral_id")),
    _gap_indices(_hex_lattice.gapIndices())
{
}

std::unique_ptr<MooseMesh>
HexagonalSubchannelGapMesh::safeClone() const
{
  return _app.getFactory().copyConstruct(*this);
}

void
HexagonalSubchannelGapMesh::buildMesh()
{
  MeshBase & mesh = getMesh();
  mesh.clear();
  mesh.set_mesh_dimension(2);
  mesh.set_spatial_dimension(3);

  _elem_id_counter = 0;
  _node_id_counter = 0;

  const Real r = _hex_lattice.pinRadius();
  Real dz = _height / _n_axial;

  for (unsigned int i = 0; i < _n_axial; ++i)
  {
    Real zmin = i * dz;
    Real zmax = (i + 1) * dz;

    for (unsigned int i = 0; i < _hex_lattice.nInteriorGaps(); ++i)
    {
      const auto & pins = _gap_indices[i];

      // to ensure accurate areas (in case anyone wants that), we need to adjust the points
      // to not overlap with the pincells that would be here
      const auto & center1 = _pin_centers[pins.first];
      const auto & center2 = _pin_centers[pins.second];
      const Point pt1 = center1 + r * (center2 - center1).unit();
      const Point pt2 = center2 + r * (center1 - center2).unit();

      addQuadElem(pt1, pt2, zmin, zmax, _interior_id);
    }

    Real d = _hex_lattice.pinBundleSpacing() + _hex_lattice.pinRadius();

    for (unsigned int i = _hex_lattice.nInteriorGaps(); i < _gap_indices.size(); ++i)
    {
      const auto & pins = _gap_indices[i];
      int side = std::abs(pins.second) - 1;

      const auto & center1 = _pin_centers[pins.first];
      const Point pt2 = center1 + Point(d * _hex_lattice.sideTranslationX(side),
                                        d * _hex_lattice.sideTranslationY(side),
                                        0.0);
      const Point pt1 = center1 + r * (pt2 - center1).unit();

      addQuadElem(pt1, pt2, zmin, zmax, _peripheral_id);
    }
  }

  mesh.prepare_for_use();
}

void
HexagonalSubchannelGapMesh::addQuadElem(const Point & pt1,
                                        const Point & pt2,
                                        const Real & zmin,
                                        const Real & zmax,
                                        const unsigned int & id)
{
  auto elem = new Quad4;
  elem->set_id(_elem_id_counter++);
  _mesh->add_elem(elem);

  Point z0(0.0, 0.0, zmin);
  Point z1(0.0, 0.0, zmax);

  auto node_ptr0 = _mesh->add_point(pt1 + z0, _node_id_counter++);
  auto node_ptr1 = _mesh->add_point(pt2 + z0, _node_id_counter++);
  auto node_ptr2 = _mesh->add_point(pt2 + z1, _node_id_counter++);
  auto node_ptr3 = _mesh->add_point(pt1 + z1, _node_id_counter++);

  elem->set_node(0) = node_ptr0;
  elem->set_node(1) = node_ptr1;
  elem->set_node(2) = node_ptr2;
  elem->set_node(3) = node_ptr3;

  elem->subdomain_id() = id;
}
