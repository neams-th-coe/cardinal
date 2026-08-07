/********************************************************************/
/*                  SOFTWARE COPYRIGHT NOTICE                        */
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

#include "StructuredMesh.h"

#include "MooseError.h"
#include "libmesh/edge_edge2.h"
#include "libmesh/elem.h"
#include "libmesh/hex8.h"
#include "libmesh/mesh_base.h"
#include "libmesh/parallel.h"
#include "libmesh/quad4.h"
#include "libmesh/replicated_mesh.h"

namespace structured_mesh
{

StructuredMesh::StructuredMesh(const std::array<std::vector<Real>, 3> & coords,
                               unsigned int dim,
                               MeshType style)
  : _coords(coords), _dim(dim), _style(style)
{
  _n_bins = 1;
  for (unsigned int i = 0; i < 3; ++i)
  {
    // Collapsed (unused) axes have exactly one cell.
    unsigned int cells = (i < _dim) ? (_coords[i].size() - 1) : 1;
    _shape[i] = cells;
    _n_bins *= cells;
  }
}

void
StructuredMesh::setID(int32_t id)
{
  _openmc_mesh->set_id(id);
}

void
StructuredMesh::buildOpenMCMesh()
{
  if (_style == MeshType::REGULAR)
    buildRegular();
  else
    buildRectilinear();
}

void
StructuredMesh::buildRegular()
{
  auto mesh = std::make_unique<openmc::RegularMesh>();

  mesh->n_dimension_ = _dim;

  // The regular mesh tensors must be sized exactly to the number of dimensions.
  mesh->lower_left_.resize({_dim});
  mesh->width_.resize({_dim});
  for (unsigned int i = 0; i < _dim; ++i)
  {
    mesh->lower_left_(i) = _coords[i].front();
    mesh->width_(i) = _coords[i][1] - _coords[i][0];
  }

  for (unsigned int i = 0; i < 3; ++i)
    mesh->shape_[i] = static_cast<int>(_shape[i]);

  if (int err = mesh->set_grid())
    mooseError(openmc_err_msg);

  openmc::model::meshes.push_back(std::move(mesh));
  _mesh_index = openmc::model::meshes.size() - 1;
  _openmc_mesh =
      dynamic_cast<openmc::StructuredMesh *>(openmc::model::meshes[_mesh_index].get());
}

void
StructuredMesh::buildRectilinear()
{
  auto mesh = std::make_unique<openmc::RectilinearMesh>();

  // OpenMC's rectilinear mesh is always 3-D internally; unused axes get a single cell.
  mesh->n_dimension_ = 3;

  std::array<std::vector<double>, 3> grid;
  for (unsigned int i = 0; i < 3; ++i)
    grid[i] = (i < _dim) ? _coords[i] : std::vector<double>{0.0, 1.0};

  mesh->grid_ = grid;

  if (int err = mesh->set_grid())
    mooseError(openmc_err_msg);

  openmc::model::meshes.push_back(std::move(mesh));
  _mesh_index = openmc::model::meshes.size() - 1;
  _openmc_mesh =
      dynamic_cast<openmc::StructuredMesh *>(openmc::model::meshes[_mesh_index].get());
}

void
StructuredMesh::buildLibMeshMesh(const libMesh::Parallel::Communicator & comm)
{
  _libmesh_mesh = std::make_unique<libMesh::ReplicatedMesh>(comm);
  _libmesh_mesh->set_mesh_dimension(_dim);
  // Keep the manually-assigned (bin == element id) numbering.
  _libmesh_mesh->allow_renumbering(false);

  std::array<unsigned int, 3> n_nodes;
  for (unsigned int i = 0; i < 3; ++i)
    n_nodes[i] = _coords[i].size();

  // Collapsed (unused) axes still need a single node plane.
  std::array<unsigned int, 3> alloc_nodes;
  alloc_nodes[0] = n_nodes[0];
  alloc_nodes[1] = (_dim >= 2) ? n_nodes[1] : 1;
  alloc_nodes[2] = (_dim >= 3) ? n_nodes[2] : 1;

  // Create all the nodes (x-major, then y, then z).
  for (unsigned int iz = 0; iz < alloc_nodes[2]; ++iz)
    for (unsigned int iy = 0; iy < alloc_nodes[1]; ++iy)
      for (unsigned int ix = 0; ix < alloc_nodes[0]; ++ix)
      {
        Real z_coord = (_dim >= 3) ? _coords[2][iz] : 0.0;
        Real y_coord = (_dim >= 2) ? _coords[1][iy] : 0.0;
        libMesh::Point p(_coords[0][ix], y_coord, z_coord);
        _libmesh_mesh->add_point(p);
      }

  auto node_id = [&](unsigned int ix, unsigned int iy, unsigned int iz)
  {
    return ix + alloc_nodes[0] * (iy + alloc_nodes[1] * iz);
  };

  // Create the elements in bin order (x fastest, then y, then z) so element id == bin.
  unsigned int bin = 0;
  for (unsigned int iz = 0; iz < _shape[2]; ++iz)
    for (unsigned int iy = 0; iy < _shape[1]; ++iy)
      for (unsigned int ix = 0; ix < _shape[0]; ++ix, ++bin)
      {
        libMesh::Elem * elem = nullptr;
        if (_dim == 3)
        {
          elem = new libMesh::Hex8;
          elem->set_node(0, _libmesh_mesh->node_ptr(node_id(ix, iy, iz)));
          elem->set_node(1, _libmesh_mesh->node_ptr(node_id(ix + 1, iy, iz)));
          elem->set_node(2, _libmesh_mesh->node_ptr(node_id(ix + 1, iy + 1, iz)));
          elem->set_node(3, _libmesh_mesh->node_ptr(node_id(ix, iy + 1, iz)));
          elem->set_node(4, _libmesh_mesh->node_ptr(node_id(ix, iy, iz + 1)));
          elem->set_node(5, _libmesh_mesh->node_ptr(node_id(ix + 1, iy, iz + 1)));
          elem->set_node(6, _libmesh_mesh->node_ptr(node_id(ix + 1, iy + 1, iz + 1)));
          elem->set_node(7, _libmesh_mesh->node_ptr(node_id(ix, iy + 1, iz + 1)));
        }
        else if (_dim == 2)
        {
          elem = new libMesh::Quad4;
          elem->set_node(0, _libmesh_mesh->node_ptr(node_id(ix, iy, 0)));
          elem->set_node(1, _libmesh_mesh->node_ptr(node_id(ix + 1, iy, 0)));
          elem->set_node(2, _libmesh_mesh->node_ptr(node_id(ix + 1, iy + 1, 0)));
          elem->set_node(3, _libmesh_mesh->node_ptr(node_id(ix, iy + 1, 0)));
        }
        else // dim == 1
        {
          elem = new libMesh::Edge2;
          elem->set_node(0, _libmesh_mesh->node_ptr(node_id(ix, 0, 0)));
          elem->set_node(1, _libmesh_mesh->node_ptr(node_id(ix + 1, 0, 0)));
        }

        _libmesh_mesh->add_elem(elem);
        elem->set_id(bin);
      }

  _libmesh_mesh->prepare_for_use();
}

Real
StructuredMesh::binVolume(unsigned int bin) const
{
  return _openmc_mesh->volume(bin);
}

Point
StructuredMesh::binCentroid(unsigned int bin) const
{
  const auto ijk = _openmc_mesh->get_indices_from_bin(bin);

  Point centroid(0.0, 0.0, 0.0);
  for (unsigned int i = 0; i < _dim; ++i)
  {
    Real lo = _coords[i][ijk[i] - 1];
    Real hi = _coords[i][ijk[i]];
    centroid(i) = 0.5 * (lo + hi);
  }
  return centroid;
}

unsigned int
StructuredMesh::addScalarField(const std::string & name)
{
  _field_names.push_back(name);
  _fields.emplace_back(_n_bins, 0.0);
  return _fields.size() - 1;
}

void
StructuredMesh::setBinValues(unsigned int field, const std::vector<Real> & values)
{
  mooseAssert(values.size() == _n_bins, "Values array size must match the number of bins");
  _fields[field] = values;
}

const std::vector<Real> &
StructuredMesh::getBinValues(unsigned int field) const
{
  return _fields[field];
}

} // namespace structured_mesh