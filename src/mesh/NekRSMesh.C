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
#include "libmesh/cell_hex8.h"
#include "libmesh/cell_hex27.h"
#include "nekrs.hpp"

registerMooseObject("MooseApp", NekRSMesh);

template <>
InputParameters
validParams<NekRSMesh>()
{
  InputParameters params = validParams<MooseMesh>();
  params.addParam<std::vector<int>>("boundary", "Boundary ID(s) through which nekRS will be coupled to MOOSE");
  params.addParam<bool>("volume", false, "Whether the nekRS volume will be coupled to MOOSE");
  params.addParam<MooseEnum>("order", getNekOrderEnum(), "Order of the surface interpolation between nekRS and MOOSE");
  params.addParam<bool>("verbose", false, "Whether to print mesh generation results to screen");
  params.addRangeCheckedParam<Real>("scaling", 1.0, "scaling > 0.0", "Scaling factor to apply to the mesh");
  return params;
}

NekRSMesh::NekRSMesh(const InputParameters & parameters) :
  MooseMesh(parameters),
  _volume(getParam<bool>("volume")),
  _boundary(isParamValid("boundary") ? &getParam<std::vector<int>>("boundary") : nullptr),
  _order(getParam<MooseEnum>("order").getEnum<order::NekOrderEnum>()),
  _verbose(getParam<bool>("verbose")),
  _scaling(getParam<Real>("scaling")),
  _n_surface_elems(0),
  _n_volume_elems(0)
{
  if (!_boundary && !_volume)
    mooseError("'NekRSMesh' requires at least 'volume = true' or a list of IDs in 'boundary'!");

  if (_boundary && _boundary->empty())
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
      "fixed at its initial condition, and the heat flux and power transferred to nekRS will be unused.");

  // For boundary-based coupling, we should check that the boundary supplied is valid
  // and that the correct heat flux boundary conditions are applied in nekRS
  if (_boundary)
  {
    const auto & filename = getMooseApp().getInputFileName();
    int first_invalid_id, n_boundaries;
    bool valid_ids = nekrs::mesh::validBoundaryIDs(*_boundary, first_invalid_id, n_boundaries);

    if (!valid_ids)
      mooseError("Invalid 'boundary' entry specified for 'NekRSMesh': ", first_invalid_id, "\n\n"
        "nekRS assumes the boundary IDs are ordered contiguously beginning at 1. "
        "For this problem, nekRS has ", n_boundaries, " boundaries. "
        "Did you enter a valid 'boundary' in '" + filename + "'?");

    // check that the Nek problem has a flux boundary condition set on all of the
    // NekRSMesh's boundaries. To avoid throwing this
    // error for test cases where we have a [TEMPERATURE] block but set its solve
    // to 'none', we also check whether we're actually computing for the temperature.
    if (has_temperature_solve)
    {
      for (const auto & b : *_boundary)
        if (!nekrs::mesh::isHeatFluxBoundary(b))
        {
          const std::string type = nekrs::mesh::temperatureBoundaryType(b);
          mooseError("In order to send a boundary heat flux to nekRS, you must have a flux condition "
            "for each 'boundary' set in 'NekRSMesh'!\nBoundary " + std::to_string(b) + " is of type '" +
            type + "' instead of 'fixedGradient'.");
        }
    }
  }

  // For volume-based coupling, we should check that there is a udf function providing
  // the source for the passive scalar equations (this is the analogue of the boundary
  // condition check for boundary-based coupling). NOTE: This check is imperfect, because
  // even if there is a source kernel, we cannot tell _which_ passive scalar equation that
  // it is applied to (we have source kernels for the RANS passive scalar equations, for instance).
  if (_volume)
    if (has_temperature_solve && !nekrs::hasHeatSourceKernel())
      mooseError("In order to send a heat source to nekRS, you must have an OCCA kernel "
        "for the source in the passive scalar equations!");

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

int
NekRSMesh::numQuadraturePoints1D() const
{
  return _order + 2;
}

int
NekRSMesh::nekNumQuadraturePoints1D() const
{
  return _nek_polynomial_order + 1;
}

void
NekRSMesh::initializeMeshParams()
{
  _nek_polynomial_order = nekrs::mesh::polynomialOrder();

  /**
   * The libMesh face numbering for a 3-D hexagonal element is
   *            0
   *            ^    3
   *            |   /
   *         o--------o
   *        /:  | /  /|
   *       / :  |/  / |
   *      /  :     /  |
   *     o--------o  -|-> 2
   *  4<-|-  o....|...o
   *     |  .     |  /
   *     | .  /|  | /
   *     |.  / |  |/
   *     o--------o
   *       /   |
   *      1    5
   *
   * but for nekRS it is
   *            3
   *            ^    5
   *            |   /
   *         o--------o
   *        /:  | /  /|
   *       / :  |/  / |
   *      /  :     /  |
   *     o--------o  -|-> 2
   *  4<-|-  o....|...o
   *     |  .     |  /
   *     | .  /|  | /
   *     |.  / |  |/
   *     o--------o
   *       /   |
   *      0    1

   */
  _side_index = {1, 5, 2, 0, 4, 3};

  switch (_order)
  {
    case order::first:
      _n_vertices_per_surface = 4;
      _n_vertices_per_volume = 8;

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
      _bnd_node_index = {0, 1, 3, 2};

      /** The libMesh node number for Hex 8 is
       *         3        2
       *         o--------o
       *        /:       /|
       *       / :      / |
       *    0 /  :   1 /  |
       *     o--------o   |
       *     |   o....|...o 6
       *     |  .7    |  /
       *     | .      | /
       *     |.       |/
       *     o--------o
       *     4        5
       *
       * but for nekRS it is
       *
       *         6        7
       *         o--------o
       *        /:       /|
       *       / :      / |
       *    2 /  :   3 /  |
       *     o--------o   |
       *     |   o....|...o 5
       *     |  .4    |  /
       *     | .      | /
       *     |.       |/
       *     o--------o
       *     0        1
       */
       _vol_node_index = {2, 3, 7, 6, 0, 1, 5, 4};

      break;
    case order::second:
      _n_vertices_per_surface = 9;
      _n_vertices_per_volume = 27;

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
      _bnd_node_index = {0, 2, 8, 6, 1, 5, 7, 3, 4};

      /** The libMesh node numbering for Hex 27 is
       *
       *
       *               3              10             2
       *              o--------------o--------------o
       *             /:             /              /|
       *            / :            /              / |
       *           /  :           /              /  |
       *        11/   :        20/             9/   |
       *         o--------------o--------------o    |
       *        /     :        /              /|    |
       *       /    15o       /    23o       / |  14o
       *      /       :      /              /  |   /|
       *    0/        :    8/             1/   |  / |
       *    o--------------o--------------o    | /  |
       *    |         :    |   26         |    |/   |
       *    |  24o    :    |    o         |  22o    |
       *    |         :    |       18     |   /|    |
       *    |        7o....|.........o....|../.|....o
       *    |        .     |              | /  |   / 6
       *    |       .    21|            13|/   |  /
       * 12 o--------------o--------------o    | /
       *    |     .        |              |    |/
       *    |  19o         | 25o          |    o
       *    |   .          |              |   / 17
       *    |  .           |              |  /
       *    | .            |              | /
       *    |.             |              |/
       *    o--------------o--------------o
       *    4              16              5
       *
       * but for nekRS it is
       *
       *               24             25             26
       *              o--------------o--------------o
       *             /:             /              /|
       *            / :            /              / |
       *           /  :           /              /  |
       *        15/   :        16/            17/   |
       *         o--------------o--------------o    |
       *        /     :        /              /|    |
       *       /    21o       /    22o       / |  23o
       *      /       :      /              /  |   /|
       *    6/        :    7/             8/   |  / |
       *    o--------------o--------------o    | /  |
       *    |         :    |   13         |    |/   |
       *    |  12o    :    |    o         |  14o    |
       *    |         :    |       19     |   /|    |
       *    |       18o....|.........o....|../.|....o
       *    |        .     |              | /  |   / 20
       *    |       .     4|             5|/   |  /
       *  3 o--------------o--------------o    | /
       *    |     .        |              |    |/
       *    |   9o         | 10o          |    o
       *    |   .          |              |   / 11
       *    |  .           |              |  /
       *    | .            |              | /
       *    |.             |              |/
       *    o--------------o--------------o
       *    0              1              2
       */
       _vol_node_index = {6, 8, 26, 24, 0, 2, 20, 18, 7, 17, 25, 15, 3, 5, 23, 21, 1, 11, 19, 9, 16, 4, 14, 22, 12, 10, 13};

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

  _nek_n_surface_elems = nekrs::mesh::NboundaryFaces();
  _nek_n_volume_elems = nekrs::mesh::Nelements();

  _console << "Total number of volume elements: " << _nek_n_volume_elems << std::endl;
  _console << "Total number of surface elements: " << _nek_n_surface_elems << std::endl;

  // initialize the mesh mapping parameters that depend on order
  initializeMeshParams();

  // Loop through the mesh to establish a data structure (nek_boundary_coupling)
  // that holds the rank-local element ID, element-local face ID, and owning rank.
  // This data structure is used internally by nekRS during the transfer portion.
  // We must call this before the volume portion so that we can map the boundary
  // coupling to the volume coupling.
  if (_boundary)
    nekrs::mesh::storeBoundaryCoupling(*_boundary, _n_surface_elems);

  // Loop through the mesh to establish a data structure (nek_volume_coupling)
  // that holds the rank-local element ID and owning rank.
  // This data structure is used internally by nekRS during the transfer portion.
  if (_volume)
    nekrs::mesh::storeVolumeCoupling(_n_volume_elems);

  if (_boundary && !_volume)
    extractSurfaceMesh();

  if (_volume)
    extractVolumeMesh();

  addElems();

  // We're looking up the elements by id, so we can't let the ids get
  // renumbered.
  _mesh->allow_renumbering(false);

  // If we have a DistributedMesh then we've already partitioned the
  // elements to match the nekrs mesh, and libMesh shouldn't try to
  // improve on that.  We won't ever be doing any element deletion or
  // coarsening, but we *have* neglected to partition the nodes, so
  // we'll rely on libMesh's "critical" partitioning to do that.
  if (!_mesh->is_replicated())
    _mesh->skip_noncritical_partitioning(true);

  _mesh->prepare_for_use();

  _console << "Done preparing nekRS MooseMesh." << std::endl;
}

void
NekRSMesh::addElems()
{
  BoundaryInfo & boundary_info = _mesh->get_boundary_info();

  for (int e = 0; e < _n_elems; e++)
  {
    auto elem = (this->*_new_elem)();
    _mesh->add_elem(elem);

    if (_verbose)
      _console << std::endl;

    // add one point for each vertex of the face element
    for (int n = 0; n < _n_vertices_per_elem; n++)
    {
      int node = (*_node_index)[n];

      auto node_offset = e * _n_vertices_per_elem + node;
      Point p(_x[node_offset], _y[node_offset], _z[node_offset]);
      p *= _scaling;

      if (_verbose)
        _console << "Adding point: " << p << std::endl;

      auto node_ptr = _mesh->add_point(p);
      elem->set_node(n) = node_ptr;
      elem->processor_id() = _elem_processor_id(e);
    }

    // add sideset IDs to the mesh if we have volume coupling (this only adds the
    // sidesets associated with the coupling)
    if (_volume && _boundary)
    {
      int n_faces_on_boundary = nekrs::mesh::facesOnBoundary(e);
      for (int f = 0; f < n_faces_on_boundary; ++f)
      {
        // get the sideset ID and local face ID
        int elem_local_face;
        int boundary_id;
        nekrs::mesh::faceSideset(e, f, elem_local_face, boundary_id);

        // add this side to the appropriate boundary ID based on the elem-local face ID
        boundary_info.add_side(elem, _side_index[elem_local_face], boundary_id);
      }
    }
  }
}

void
NekRSMesh::extractSurfaceMesh()
{
  _console << "Building surface coupling mesh...";

  _x = (double*) malloc(_n_surface_elems * _n_vertices_per_surface * sizeof(double));
  _y = (double*) malloc(_n_surface_elems * _n_vertices_per_surface * sizeof(double));
  _z = (double*) malloc(_n_surface_elems * _n_vertices_per_surface * sizeof(double));

  // Find the global vertex IDs that are on the _boundary. Note that nekRS performs a
  // global communciation here such that each nekRS process has knowledge of all the
  // boundary information.
  nekrs::mesh::faceVertices(_order, _x, _y, _z);

  _console << " Boundary " << Moose::stringify(*_boundary) << " contains " << _n_surface_elems <<
    " of the total of " << _nek_n_surface_elems << " nekRS surface elements" << std::endl;

  _new_elem = &NekRSMesh::boundaryElem;
  _n_elems = _n_surface_elems;
  _n_vertices_per_elem = _n_vertices_per_surface;
  _node_index = &_bnd_node_index;
  _elem_processor_id = nekrs::mesh::BoundaryElemProcessorID;
}

void
NekRSMesh::extractVolumeMesh()
{
  _console << "Building volume coupling mesh...";

  // nekRS has already performed a global operation such that all processes know the
  // toal number of volume elements.
  _x = (double*) malloc(_n_volume_elems * _n_vertices_per_volume * sizeof(double));
  _y = (double*) malloc(_n_volume_elems * _n_vertices_per_volume * sizeof(double));
  _z = (double*) malloc(_n_volume_elems * _n_vertices_per_volume * sizeof(double));

  // Find the global vertex IDs in the volume. Note that nekRS performs a
  // global communciation here such that each nekRS process has knowledge of all the
  // volume information.
  nekrs::mesh::volumeVertices(_order, _x, _y, _z);

  _console << " Volume contains " << _n_volume_elems <<
    " of the total of " << _nek_n_volume_elems << " nekRS volume elements" << std::endl;

  _new_elem = &NekRSMesh::volumeElem;
  _n_elems = _n_volume_elems;
  _n_vertices_per_elem = _n_vertices_per_volume;
  _node_index = &_vol_node_index;
  _elem_processor_id = nekrs::mesh::VolumeElemProcessorID;
}

Elem *
NekRSMesh::boundaryElem() const
{
  switch (_order)
  {
    case order::first:
      return new Quad4;
      break;
    case order::second:
      return new Quad9;
      break;
    default:
      mooseError("Unhandled 'NekOrderEnum' in 'NekRSMesh'!");
  }
}

Elem *
NekRSMesh::volumeElem() const
{
  switch (_order)
  {
    case order::first:
      return new Hex8;
      break;
    case order::second:
      return new Hex27;
      break;
    default:
      mooseError("Unhandled 'NekOrderEnum' in 'NekRSMesh'!");
  }
}
