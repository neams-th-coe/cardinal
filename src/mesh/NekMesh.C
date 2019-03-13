//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NekMesh.h"
#include "NekInterface.h"

#include "libmesh/face_quad4.h"
#include "libmesh/face_tri3.h"

registerMooseObject("MooseApp", NekMesh);

template <>
InputParameters
validParams<NekMesh>()
{
  InputParameters params = validParams<MooseMesh>();
  return params;
}

NekMesh::NekMesh(const InputParameters & parameters) : MooseMesh(parameters) {}

std::unique_ptr<MooseMesh>
NekMesh::safeClone() const
{
  return libmesh_make_unique<NekMesh>(*this);
}

void
NekMesh::buildMesh()
{
  // _mesh = _app.getNekMesh();

  Nek5000::FORTRAN_CALL(nek_pointscloud)();

  auto num_elems = Nek5000::tot_surf_.nw_dbt;

  std::cout << "Total number of elements: " << num_elems << std::endl;

  // Here's how this works:
  // We are reading Quad4s so each one has 4 nodes
  // So loop over the elements and pull out the nodes... easy

  for (unsigned int e = 0; e < num_elems; e++)
  {
    auto elem = new Quad4;
    _mesh->add_elem(elem);

    // Get the nodes for this element out
    for (unsigned int n = 0; n < 4; n++)
    {
      auto node_offset = (e * 4) + n;

      Point p(Nek5000::point_cloudx_.pc_x[node_offset],
              Nek5000::point_cloudy_.pc_y[node_offset],
              Nek5000::point_cloudz_.pc_z[node_offset]);

      std::cout << "Adding point: " << p << std::endl;

      auto node_ptr = _mesh->add_point(p);

      elem->set_node(n) = node_ptr;
    }
  }

  _mesh->prepare_for_use();
}
