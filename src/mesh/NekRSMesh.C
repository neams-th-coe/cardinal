//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NekRSMesh.h"
#include "NekInterface.h"
#include "libmesh/face_quad4.h"
#include "libmesh/face_quad9.h"
#include "nekrs.hpp"

registerMooseObject("MooseApp", NekRSMesh);

template <>
InputParameters
validParams<NekRSMesh>()
{
  InputParameters params = validParams<MooseMesh>();
  params.addRequiredParam<std::vector<int>>("boundary", "Boundary ID(s) through which nekRS will be coupled to MOOSE");
  params.addParam<MooseEnum>("order", getNekOrderEnum(), "Order of the surface interpolation between nekRS and MOOSE");
  params.addParam<bool>("verbose", false, "Whether to print mesh generation results to screen");
  params.addRangeCheckedParam<Real>("scaling", 1.0, "scaling > 0.0", "Scaling factor to apply to the mesh");
  return params;
}

NekRSMesh::NekRSMesh(const InputParameters & parameters) :
  MooseMesh(parameters),
  _boundary(getParam<std::vector<int>>("boundary")),
  _order(getParam<MooseEnum>("order").getEnum<surface::NekOrderEnum>()),
  _verbose(getParam<bool>("verbose")),
  _scaling(getParam<Real>("scaling")),
  _n_surface_elems(0)
{
  if (_boundary.size() == 0)
    paramError("boundary", "The length of 'boundary' must be greater than zero!");

  // While we don't require nekRS to actually _solve_ for the temperature, we should
  // print a warning if there is no temperature solve. For instance, the check in
  // NekApp makes sure that we have a [TEMPERATURE] block in the nekRS input file, but we
  // might still toggle the solver off by setting 'solver = none'. Warn the user if
  // the solve is turned off because this is really only a testing feature.
  bool has_temperature_solve = nekrs::hasTemperatureSolve();
  if (!has_temperature_solve)
    mooseWarning("By setting 'solver = none' for temperature in the .par file, nekRS "
      "will not solve for temperature.\n\nThe temperature transferred to MOOSE will remain "
      "fixed at its initial condition, and the heat flux transferred to nekRS will be unused.");

  const auto & filename = getMooseApp().getInputFileName();

  // check that each specified boundary is within the range [1, n_fluid_boundaries]
  // that nekRS recognizes for its problem
  int first_invalid_id, n_boundaries;
  bool valid_ids = nekrs::mesh::validBoundaryIDs(_boundary, first_invalid_id, n_boundaries);

  if (!valid_ids)
    mooseError("Invalid 'boundary' entry specified for 'NekRSMesh': ", first_invalid_id, "\n\n"
      "nekRS assumes the boundary IDs are ordered contiguously beginning at 1. "
      "For this problem, nekRS has ", n_boundaries, " boundaries. "
      "Did you enter a valid 'boundary' in '" + filename + "'?");

  // check that the Nek problem has a flux boundary condition set on all of the
  // NekRSMesh's boundaries. Because NekProblem::solve() always does the transfer
  // in both directions, we don't need any special corner cases to cover if the
  // data transfer were only happening in one direction. To avoid throwing this
  // error for test cases where we have a [TEMPERATURE] block but set its solve
  // to 'none', we also check whether we're actually computing for the temperature.
  if (has_temperature_solve)
  {
    for (const auto & b : _boundary)
      if (!nekrs::mesh::isHeatFluxBoundary(b))
      {
        const std::string type = nekrs::mesh::temperatureBoundaryType(b);
        mooseError("In order to send a boundary heat flux to nekRS, you must have a flux condition "
          "for each 'boundary' set in 'NekRSMesh'!\nBoundary " + std::to_string(b) + " is of type '" +
          type + "' instead of 'fixedGradient'.");
      }
  }

  // nekRS will only ever support 3-D meshes. Just to be sure that this remains
  // the case for future Cardinal developers, throw an error if the mesh isn't 3-D
  // (since this would affect how we construct the mesh here).
  int dimension = nekrs::mesh::dim();
  if (dimension != 3)
    mooseError("'NekRSMesh' assumes that the nekRS mesh dimension is 3!\n\nYour mesh is "
      "dimension " + std::to_string(dimension) + ".");
}

NekRSMesh::~NekRSMesh()
{
  if (_x) free(_x);
  if (_y) free(_y);
  if (_z) free(_z);

  nekrs::mesh::freeMesh();
}

std::unique_ptr<MooseMesh>
NekRSMesh::safeClone() const
{
  return libmesh_make_unique<NekRSMesh>(*this);
}

const surface::NekOrderEnum &
NekRSMesh::order() const
{
  return _order;
}

const int
NekRSMesh::numQuadraturePoints1D() const
{
  return _order + 2;
}

const int
NekRSMesh::nekNumQuadraturePoints1D() const
{
  return _nek_polynomial_order + 1;
}

const int &
NekRSMesh::numSurfaceElems() const
{
  return _n_surface_elems;
}

const int &
NekRSMesh::nekNumSurfaceElems() const
{
  return _nek_n_surface_elems;
}

const int &
NekRSMesh::numVerticesPerFace() const
{
  return _n_vertices_per_face;
}

const std::vector<int> &
NekRSMesh::boundary() const
{
  return _boundary;
}

void
NekRSMesh::initializeMeshParams()
{
  _nek_polynomial_order = nekrs::mesh::polynomialOrder();

  switch (_order)
  {
    case surface::first:
      _n_vertices_per_face = 4;

      /** The libMesh node numbering for Quad 4 is
       *
       *  3 -- 2
       *  |    |
       *  0 -- 1
       *
       *  but for nekRS it is
       *
       *  2 -- 3
       *  |    |
       *  0 -- 1
       **/
      _node_index = {0, 1, 3, 2};
      _gll_index = {0, 1, 3, 2};

      break;
    case surface::second:
      _n_vertices_per_face = 9;

      /** The libMesh node numbering for Quad 9 is
       *
       *  3 - 6 - 2
       *  |       |
       *  7   8   5
       *  |       |
       *  0 - 4 - 1
       *
       *  but for nekRS it is
       *
       *  6 - 7 - 8
       *  |       |
       *  3   4   5
       *  |       |
       *  0 - 1 - 2
       **/
      _node_index = {0, 2, 8, 6, 1, 5, 7, 3, 4};
      _gll_index = {0, 4, 1, 7, 8, 5, 3, 6, 2};

      break;
    default:
      mooseError("Unhandled 'NekOrderEnum' in 'NekRSMesh'!");
  }
}

void
NekRSMesh::buildMesh()
{
  _console << "Building nekRS mesh as an order " << (_order + 1) << " MooseMesh..." << std::endl;

  if (_scaling != 1.0)
  {
    std::string size = _scaling > 1.0 ? "larger" : "smaller";
    _console << "Data transfers will be done with a mesh " << Moose::stringify(_scaling) <<
      " times " << size << " than nekRS's mesh" << std::endl;
  }

  _console << "Total number of volume elements: " << nekrs::mesh::Nelements() << std::endl;

  // initialize the mesh mapping parameters that depend on order
  initializeMeshParams();

  // nekRS has already performed a global operation such that all processes know the
  // total number of faces that are on a nekRS boundary. 'nek_n_surface_elems' is the
  // maximum number of surface elements that we might want to communicate fields on
  // with MOOSE (i.e. if 'boundary' was set to _all_ of the boundaries in the nekRS model).
  _nek_n_surface_elems = nekrs::mesh::NboundaryFaces();
  _x = (float*) malloc(_nek_n_surface_elems * _n_vertices_per_face * sizeof(float));
  _y = (float*) malloc(_nek_n_surface_elems * _n_vertices_per_face * sizeof(float));
  _z = (float*) malloc(_nek_n_surface_elems * _n_vertices_per_face * sizeof(float));

  // Find the global vertex IDs that are on the _boundary. Note that nekRS performs a
  // global communciation here such that each nekRS process has knowledge of all the
  // boundary information.
  nekrs::mesh::faceVertices(_boundary, _order, _x, _y, _z, _n_surface_elems);

  _console << "Boundary " << Moose::stringify(_boundary) << " contains " << _n_surface_elems <<
    " of the total of " << _nek_n_surface_elems << " nekRS surface elements" << std::endl;

  for (unsigned int e = 0; e < _n_surface_elems; e++)
  {
    Elem* elem;

    switch (_order)
    {
      case surface::first:
        elem = new Quad4;
        break;
      case surface::second:
        elem = new Quad9;
        break;
      default:
        mooseError("Unhandled 'NekOrderEnum' in 'NekRSMesh'!");
    }

    _mesh->add_elem(elem);

    if (_verbose)
      _console << std::endl;

    // add one point for each vertex of the face element
    for (unsigned int n = 0; n < _n_vertices_per_face; n++)
    {
      int node = _node_index[n];

      auto node_offset = e * _n_vertices_per_face + node;
      Point p(_x[node_offset], _y[node_offset], _z[node_offset]);
      p *= _scaling;

      if (_verbose)
        _console << "Adding point: " << p << std::endl;

      auto node_ptr = _mesh->add_point(p);
      elem->set_node(n) = node_ptr;
    }
  }

  _mesh->prepare_for_use();

  _console << "Done preparing nekRS MooseMesh." << std::endl;
}
