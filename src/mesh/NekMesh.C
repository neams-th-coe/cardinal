//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NekMesh.h"
//#include "NekInterface.h"

#include "libmesh/face_quad4.h"
#include "libmesh/face_tri3.h"
#include "nekrs.hpp"
#include "nekInterface/nekInterfaceAdapter.hpp"

registerMooseObject("MooseApp", NekMesh);

InputParameters
NekMesh::validParams()
{
  InputParameters params = MooseMesh::validParams();
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
  //Nek5000::FORTRAN_CALL(nek_pointscloud)();

  
  double *n_nekrs = &nekData.cbscnrs[0];
  int num_elems =  0;
  num_elems = (int) n_nekrs[0];

  double *pc_x = &nekData.cbscnrs[1];
  double *pc_y = &nekData.cbscnrs[1+4*num_elems];
  double *pc_z = &nekData.cbscnrs[1+2*4*num_elems];

  std::cout << "Total number of elements: " << num_elems << std::endl;

  // Here's how this works:
  // We are reading Quad4s so each one has 4 nodes
  // So loop over the elements and pull out the nodes... easy

  for (int e = 0; e < num_elems; e++)
  {
    auto elem = new Quad4;
    _mesh->add_elem(elem);

    // Get the nodes for this element out
    for (unsigned int n = 0; n < 4; n++)
    {
      auto node_offset = (e * 4) + n;

      Point p(pc_x[node_offset],
              pc_y[node_offset],
              pc_z[node_offset]);

      std::cout << "Adding point: " << p << std::endl;

      auto node_ptr = _mesh->add_point(p);

      elem->set_node(n) = node_ptr;
    }
  }
  std::cout << "Test completion";
  _mesh->prepare_for_use();
  
}
