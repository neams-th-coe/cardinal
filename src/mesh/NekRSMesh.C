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

#ifdef ENABLE_NEK_COUPLING

#include "NekRSMesh.h"
#include "libmesh/face_quad4.h"
#include "libmesh/face_quad9.h"
#include "libmesh/cell_hex8.h"
#include "libmesh/cell_hex27.h"
#include "nekrs.hpp"
#include "CardinalUtils.h"
#include "VariadicTable.h"

registerMooseObject("CardinalApp", NekRSMesh);

InputParameters
NekRSMesh::validParams()
{
  InputParameters params = MooseMesh::validParams();
  params.addParam<std::vector<int>>("boundary",
                                    "Boundary ID(s) through which nekRS will be coupled to MOOSE");
  params.addParam<bool>("volume", false, "Whether the nekRS volume will be coupled to MOOSE");
  params.addParam<bool>("exact", false, "Whether the mesh mirror is an exact replica of the NekRS mesh");
  params.addParam<MooseEnum>(
      "order", getNekOrderEnum(), "Order of the mesh interpolation between nekRS and MOOSE");
  params.addRangeCheckedParam<Real>(
      "scaling", 1.0, "scaling > 0.0", "Scaling factor to apply to the mesh");
  params.addParam<unsigned int>("fluid_block_id", 0, "Subdomain ID to use for the fluid mesh mirror");
  params.addParam<unsigned int>("solid_block_id", 1, "Subdomain ID to use for the solid mesh mirror");
  params.addClassDescription(
      "Construct a mirror of the NekRS mesh in boundary and/or volume format");
  return params;
}

NekRSMesh::NekRSMesh(const InputParameters & parameters)
  : MooseMesh(parameters),
    _volume(getParam<bool>("volume")),
    _boundary(isParamValid("boundary") ? &getParam<std::vector<int>>("boundary") : nullptr),
    _order(getParam<MooseEnum>("order").getEnum<order::NekOrderEnum>()),
    _exact(getParam<bool>("exact")),
    _scaling(getParam<Real>("scaling")),
    _fluid_block_id(getParam<unsigned int>("fluid_block_id")),
    _solid_block_id(getParam<unsigned int>("solid_block_id")),
    _n_surface_elems(0),
    _n_volume_elems(0)
{
  if (_exact && _order != order::first)
    mooseError("When building an exact mesh mirror, the 'order' must be FIRST!");

  if (!_boundary && !_volume)
    mooseError("This mesh requires at least 'volume = true' or a list of IDs in 'boundary'!");

  if (_boundary && _boundary->empty())
    paramError("boundary", "The length of 'boundary' must be greater than zero!");

  // see if NekRS's mesh even exists
  if (!nekrs::isInitialized())
    mooseError("This mesh can only be used with wrapped Nek cases!\n"
               "You need to change the problem type to a Nek-wrapped problem.\n\n"
               "options: 'NekRSProblem', 'NekRSSeparateDomainProblem', 'NekRSStandaloneProblem'");

  _nek_internal_mesh = nekrs::entireMesh();

  nekrs::initializeHostMeshParameters();
  nekrs::updateHostMeshParameters();

  // nekRS will only ever support 3-D meshes. Just to be sure that this remains
  // the case for future Cardinal developers, throw an error if the mesh isn't 3-D
  // (since this would affect how we construct the mesh here).
  int dimension = nekrs::dim();
  if (dimension != 3)
    mooseError("This mesh assumes that the nekRS mesh dimension is 3!\n\nYour mesh is "
               "dimension " +
               std::to_string(dimension) + ".");

  // if doing a JIT build, the boundary information does not exist yet
  if (!nekrs::buildOnly() && _boundary)
  {
    int first_invalid_id, n_boundaries;
    bool valid_ids = nekrs::validBoundaryIDs(*_boundary, first_invalid_id, n_boundaries);

    if (!valid_ids)
      mooseError("Invalid 'boundary' entry: ",
                 first_invalid_id,
                 "\n\n"
                 "nekRS assumes the boundary IDs are ordered contiguously beginning at 1. "
                 "For this problem, nekRS has ",
                 n_boundaries,
                 " boundaries. "
                 "Did you enter a valid 'boundary'?");
  }

  _corner_indices = nekrs::cornerGLLIndices(nekrs::entireMesh()->N, _exact);
}

void
NekRSMesh::saveInitialVolMesh()
{
  // save the initial mesh structure in case we are applying displacements
  // (which are additive to the initial mesh structure)

  long ngllpts = _nek_internal_mesh->Nelements * _nek_internal_mesh->Np;

  _initial_x.resize(ngllpts,0.0);
  _initial_y.resize(ngllpts,0.0);
  _initial_z.resize(ngllpts,0.0);

  memcpy(_initial_x.data(), _nek_internal_mesh->x, ngllpts * sizeof(double));
  memcpy(_initial_y.data(), _nek_internal_mesh->y, ngllpts * sizeof(double));
  memcpy(_initial_z.data(), _nek_internal_mesh->z, ngllpts * sizeof(double));
}

void
NekRSMesh::initializePreviousDisplacements()
{
  long disp_length = _volume ? _n_vertices_per_volume * _n_volume_elems
           : _n_vertices_per_surface * _n_surface_elems;
  _prev_disp_x.resize(disp_length,0.0);
  _prev_disp_y.resize(disp_length,0.0);
  _prev_disp_z.resize(disp_length,0.0);
}

void
NekRSMesh::printMeshInfo() const
{
  _console << "\nNekRS mesh mapping to MOOSE:" << std::endl;
  VariadicTable<std::string, int, std::string, int, int> vt(
      {"", "Order", "Boundaries", "# Side Elems", "# Volume Elems"});

  std::vector<int> nek_bids;
  for (int i = 1; i <= nekrs::NboundaryID(); ++i)
    nek_bids.push_back(i);

  vt.addRow("NekRS mesh",
            nekrs::polynomialOrder(),
            Moose::stringify(nek_bids),
            _nek_n_surface_elems,
            _nek_n_volume_elems);

  std::string boundaries = "";
  if (_boundary)
    boundaries = Moose::stringify(*_boundary);
  else
    boundaries = Moose::stringify(nek_bids);
  vt.addRow("NekRS mirror", _order + 1, boundaries, _n_surface_elems * _n_build_per_surface_elem,
    _n_volume_elems * _n_build_per_volume_elem);

  vt.print(_console);
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
  _nek_polynomial_order = nekrs::polynomialOrder();
  _n_build_per_surface_elem = _exact ? _nek_polynomial_order * _nek_polynomial_order : 1;
  _n_build_per_volume_elem = _exact ? std::pow(_nek_polynomial_order, 3) : 1;

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
      _vol_node_index = {6,  8,  26, 24, 0,  2, 20, 18, 7,  17, 25, 15, 3, 5,
                         23, 21, 1,  11, 19, 9, 16, 4,  14, 22, 12, 10, 13};

      break;
    default:
      mooseError("Unhandled 'NekOrderEnum' in 'NekRSMesh'!");
  }
}

void
NekRSMesh::buildDummyMesh()
{
  int e = 1;
  auto elem = new Quad4;
  elem->set_id() = e;
  elem->processor_id() = 0;
  _mesh->add_elem(elem);

  Point pt1(0.0, 0.0, 0.0);
  Point pt2(1.0, 0.0, 0.0);
  Point pt3(1.0, 1.0, 0.0);
  Point pt4(0.0, 1.0, 0.0);

  elem->set_node(0) = _mesh->add_point(pt1);
  elem->set_node(1) = _mesh->add_point(pt2);
  elem->set_node(2) = _mesh->add_point(pt3);
  elem->set_node(3) = _mesh->add_point(pt4);

  _mesh->prepare_for_use();
}

void
NekRSMesh::storeBoundaryCoupling()
{
  int rank = nekrs::commRank();
  int max_possible_surfaces = _nek_internal_mesh->NboundaryFaces;

  int * etmp = (int *)malloc(max_possible_surfaces * sizeof(int));
  int * ftmp = (int *)malloc(max_possible_surfaces * sizeof(int));
  int * ptmp = (int *)malloc(max_possible_surfaces * sizeof(int));
  int * btmp = (int *)malloc(max_possible_surfaces * sizeof(int));
  int * element = (int *)malloc(max_possible_surfaces * sizeof(int));
  int * face = (int *)malloc(max_possible_surfaces * sizeof(int));
  int * process = (int *)malloc(max_possible_surfaces * sizeof(int));
  int * boundary_id = (int *)malloc(max_possible_surfaces * sizeof(int));

  // number of faces on boundary of interest for this process
  int Nfaces = 0;

  int d = 0;
  for (int i = 0; i < _nek_internal_mesh->Nelements; ++i)
  {
    for (int j = 0; j < _nek_internal_mesh->Nfaces; ++j)
    {
      int face_id = _nek_internal_mesh->EToB[i * _nek_internal_mesh->Nfaces + j];

      if (std::find(_boundary->begin(), _boundary->end(), face_id) != _boundary->end())
      {
        Nfaces += 1;

        etmp[d] = i;
        ftmp[d] = j;
        ptmp[d] = rank;
        btmp[d] = face_id;
        d++;
      }
    }
  }

  // gather all the boundary face counters and make available in N
  MPI_Allreduce(&Nfaces, &_n_surface_elems, 1, MPI_INT, MPI_SUM, platform->comm.mpiComm);
  _boundary_coupling.n_faces = Nfaces;
  _boundary_coupling.total_n_faces = _n_surface_elems;

  // make available to all processes the number of faces owned by each process
  _boundary_coupling.counts.resize(nekrs::commSize());
  MPI_Allgather(
      &Nfaces, 1, MPI_INT, &_boundary_coupling.counts[0], 1, MPI_INT, platform->comm.mpiComm);

  int N_mirror_faces = Nfaces * _n_build_per_surface_elem;
  _boundary_coupling.mirror_counts.resize(nekrs::commSize());
  MPI_Allgather(
      &N_mirror_faces, 1, MPI_INT, &_boundary_coupling.mirror_counts[0], 1, MPI_INT, platform->comm.mpiComm);

  // compute the counts and displacements for face-based data exchange
  int * recvCounts = (int *)calloc(nekrs::commSize(), sizeof(int));
  int * displacement = (int *)calloc(nekrs::commSize(), sizeof(int));
  nekrs::displacementAndCounts(_boundary_coupling.counts, recvCounts, displacement, 1);

  _boundary_coupling.offset = displacement[rank];

  nekrs::allgatherv(_boundary_coupling.counts, etmp, element);
  nekrs::allgatherv(_boundary_coupling.counts, ftmp, face);
  nekrs::allgatherv(_boundary_coupling.counts, ptmp, process);
  nekrs::allgatherv(_boundary_coupling.counts, btmp, boundary_id);

  for (int i = 0; i < max_possible_surfaces; ++i)
  {
    _boundary_coupling.element.push_back(element[i]);
    _boundary_coupling.face.push_back(face[i]);
    _boundary_coupling.process.push_back(process[i]);
    _boundary_coupling.boundary_id.push_back(boundary_id[i]);
  }

  freePointer(recvCounts);
  freePointer(displacement);
  freePointer(etmp);
  freePointer(ftmp);
  freePointer(ptmp);
  freePointer(btmp);
  freePointer(element);
  freePointer(face);
  freePointer(process);
  freePointer(boundary_id);
}

void
NekRSMesh::storeVolumeCoupling()
{
  int rank = nekrs::commRank();

  _volume_coupling.n_elems = _nek_internal_mesh->Nelements;
  MPI_Allreduce(
      &_volume_coupling.n_elems, &_n_volume_elems, 1, MPI_INT, MPI_SUM, platform->comm.mpiComm);
  _volume_coupling.total_n_elems = _n_volume_elems;

  _volume_coupling.counts.resize(nekrs::commSize());
  MPI_Allgather(&_volume_coupling.n_elems,
                1,
                MPI_INT,
                &_volume_coupling.counts[0],
                1,
                MPI_INT,
                platform->comm.mpiComm);

  _volume_coupling.mirror_counts.resize(nekrs::commSize());
  int N_mirror_elems = _volume_coupling.n_elems * _n_build_per_volume_elem;
  MPI_Allgather(&N_mirror_elems,
                1,
                MPI_INT,
                &_volume_coupling.mirror_counts[0],
                1,
                MPI_INT,
                platform->comm.mpiComm);

  // Save information regarding the volume mesh coupling in terms of the process-local
  // element IDs and process ownership; the 'tmp' arrays hold the rank-local data,
  // while the other arrays hold the result of the allgatherv
  int * etmp = (int *)malloc(_n_volume_elems * sizeof(int));
  int * ptmp = (int *)malloc(_n_volume_elems * sizeof(int));
  int * btmp = (int *)malloc(_n_volume_elems * _nek_internal_mesh->Nfaces * sizeof(int));
  int * element = (int *)malloc(_n_volume_elems * sizeof(int));
  int * process = (int *)malloc(_n_volume_elems * sizeof(int));
  int * boundary = (int *)malloc(_n_volume_elems * _nek_internal_mesh->Nfaces * sizeof(int));

  for (int i = 0; i < _nek_internal_mesh->Nelements; ++i)
  {
    etmp[i] = i;
    ptmp[i] = rank;

    for (int j = 0; j < _nek_internal_mesh->Nfaces; ++j)
    {
      int id = i * _nek_internal_mesh->Nfaces + j;
      btmp[id] = _nek_internal_mesh->EToB[id];
    }
  }

  nekrs::allgatherv(_volume_coupling.counts, etmp, element, 1);
  nekrs::allgatherv(_volume_coupling.counts, ptmp, process, 1);

  int * ftmp = (int *)calloc(_n_volume_elems, sizeof(int));
  int * n_faces_on_boundary = (int *)calloc(_n_volume_elems, sizeof(int));

  int b_start = _boundary_coupling.offset;
  for (int i = 0; i < _boundary_coupling.n_faces; ++i)
  {
    int e = _boundary_coupling.element[b_start + i];
    ftmp[e] += 1;
  }

  nekrs::allgatherv(_volume_coupling.counts, ftmp, n_faces_on_boundary, 1);
  nekrs::allgatherv(_volume_coupling.counts, btmp, boundary, _nek_internal_mesh->Nfaces);

  for (int i = 0; i < _n_volume_elems * _nek_internal_mesh->Nfaces; ++i)
    _volume_coupling.boundary.push_back(boundary[i]);

  for (int i = 0; i < _n_volume_elems; ++i)
  {
    _volume_coupling.element.push_back(element[i]);
    _volume_coupling.process.push_back(process[i]);
    _volume_coupling.n_faces_on_boundary.push_back(n_faces_on_boundary[i]);
  }

  freePointer(etmp);
  freePointer(ptmp);
  freePointer(ftmp);
  freePointer(btmp);
  freePointer(element);
  freePointer(process);
  freePointer(boundary);
  freePointer(n_faces_on_boundary);
}

void
NekRSMesh::buildMesh()
{
  if (nekrs::buildOnly())
  {
    buildDummyMesh();
    return;
  }

  _nek_n_surface_elems = nekrs::NboundaryFaces();
  _nek_n_volume_elems = nekrs::Nelements();

  // initialize the mesh mapping parameters that depend on order
  initializeMeshParams();

  // Loop through the mesh to establish a data structure (_boundary_coupling)
  // that holds the rank-local element ID, element-local face ID, and owning rank.
  // This data structure is used internally by nekRS during the transfer portion.
  // We must call this before the volume portion so that we can map the boundary
  // coupling to the volume coupling.
  if (_boundary)
    storeBoundaryCoupling();

  // Loop through the mesh to establish a data structure (_volume_coupling)
  // that holds the rank-local element ID and owning rank.
  // This data structure is used internally by nekRS during the transfer portion.
  if (_volume)
    storeVolumeCoupling();

  if (_boundary && !_volume)
    extractSurfaceMesh();

  if (_volume)
    extractVolumeMesh();

  addElems();

  // We're looking up the elements by id, so we can't let the ids get
  // renumbered.
  _mesh->allow_renumbering(false);

  // If we have a DistributedMesh then:
  if (!_mesh->is_replicated())
  {
    // we've already partitioned the elements to match the nekrs
    // mesh, and libMesh shouldn't try to improve on that.  We won't
    // ever be doing any element deletion or coarsening, so we don't
    // even need libMesh's "critical" partitioning.
    _mesh->skip_partitioning(true);

    // But that means we have to update the partitioning metadata
    // ourselves
    _mesh->recalculate_n_partitions();

    // But, we haven't yet partitioned nodes, and if we tell libMesh
    // not to do that automatically then we need to do it manually
    libMesh::Partitioner::set_node_processor_ids(*_mesh);
  }

  _mesh->prepare_for_use();
}

void
NekRSMesh::addElems()
{
  BoundaryInfo & boundary_info = _mesh->get_boundary_info();
  auto nested_elems_on_face = nekrs::nestedElementsOnFace(_nek_polynomial_order);

  for (int e = 0; e < _n_elems; e++)
  {
    for (int build = 0; build < _n_moose_per_nek; ++build)
    {
      auto elem = (this->*_new_elem)();
      elem->set_id() = e * _n_moose_per_nek + build;
      elem->processor_id() = (this->*_elem_processor_id)(e);
      _mesh->add_elem(elem);

      // add one point for each vertex of the face element
      for (int n = 0; n < _n_vertices_per_elem; n++)
      {
        int node = (*_node_index)[n];

        auto node_offset = (e * _n_moose_per_nek + build) * _n_vertices_per_elem + node;
        Point p(_x[node_offset], _y[node_offset], _z[node_offset]);
        p *= _scaling;

        auto node_ptr = _mesh->add_point(p);
        elem->set_node(n) = node_ptr;
      }

      // add sideset IDs to the mesh if we have volume coupling (this only adds the
      // sidesets associated with the coupling)
      if (_volume)
      {
        for (int f = 0; f < nekrs::Nfaces(); ++f)
        {
          int b_id = boundary_id(e, f);
          if (b_id != -1 /* NekRS's setting to indicate not on a sideset */)
          {
            if (_exact)
            {
              auto faces = nested_elems_on_face[f];
              if (!std::count(faces.begin(), faces.end(), build))
                continue;
            }

            boundary_info.add_side(elem, _side_index[f], b_id);
          }
        }

        if (_phase[e * _n_moose_per_nek + build])
          elem->subdomain_id() = _solid_block_id;
        else
          elem->subdomain_id() = _fluid_block_id;
      }
    }
  }
}

void
NekRSMesh::faceVertices()
{
  int n_vertices_in_mirror = _n_build_per_surface_elem * _n_surface_elems * _n_vertices_per_surface;
  double * x = (double *) malloc(n_vertices_in_mirror * sizeof(double));
  double * y = (double *) malloc(n_vertices_in_mirror * sizeof(double));
  double * z = (double *) malloc(n_vertices_in_mirror * sizeof(double));

  nrs_t * nrs = (nrs_t *)nekrs::nrsPtr();
  int rank = nekrs::commRank();

  mesh_t * mesh;
  int Nfp_mirror;

  if (_order == 0)
  {
    // For a first-order mesh mirror, we can take a shortcut and instead just fetch the
    // corner nodes. In this case, 'mesh' is no longer a custom-build mesh copy, but the
    // actual mesh for computation
    mesh = _nek_internal_mesh;
    Nfp_mirror = 4;
  }
  else
  {
    // Create a duplicate of the solution mesh, but with the desired order of the mesh
    // interpolation. Then we can just read the coordinates of the GLL points to find the libMesh
    // node positions. We only need to do this if the NekRS mesh is not already order 2.
    if (_nek_internal_mesh->N == 2)
      mesh = _nek_internal_mesh;
    else
      mesh =
          createMesh(platform->comm.mpiComm, _order + 1, 0, nrs->cht, *(nrs->kernelInfo));

    Nfp_mirror = mesh->Nfp;
  }

  // Allocate space for the coordinates that are on this rank
  int n_vertices_on_rank = _n_build_per_surface_elem * _boundary_coupling.n_faces * Nfp_mirror;
  double * xtmp = (double *) malloc(n_vertices_on_rank * sizeof(double));
  double * ytmp = (double *) malloc(n_vertices_on_rank * sizeof(double));
  double * ztmp = (double *) malloc(n_vertices_on_rank * sizeof(double));

  int c = 0;
  for (int k = 0; k < _boundary_coupling.total_n_faces; ++k)
  {
    if (_boundary_coupling.process[k] == rank)
    {
      int i = _boundary_coupling.element[k];
      int j = _boundary_coupling.face[k];
      int offset = i * mesh->Nfaces * mesh->Nfp + j * mesh->Nfp;

      for (int build = 0; build < _n_build_per_surface_elem; ++build)
      {
        for (int v = 0; v < Nfp_mirror; ++v, ++c)
        {
          int vertex_offset = _order == 0 ? _corner_indices[build][v] : v;
          int id = mesh->vmapM[offset + vertex_offset];

          xtmp[c] = mesh->x[id];
          ytmp[c] = mesh->y[id];
          ztmp[c] = mesh->z[id];
        }
      }
    }
  }

  nekrs::allgatherv(_boundary_coupling.mirror_counts, xtmp, x, Nfp_mirror);
  nekrs::allgatherv(_boundary_coupling.mirror_counts, ytmp, y, Nfp_mirror);
  nekrs::allgatherv(_boundary_coupling.mirror_counts, ztmp, z, Nfp_mirror);

  for (int i = 0; i < n_vertices_in_mirror; ++i)
  {
    _x.push_back(x[i]);
    _y.push_back(y[i]);
    _z.push_back(z[i]);
  }

  freePointer(x);
  freePointer(y);
  freePointer(z);
  freePointer(xtmp);
  freePointer(ytmp);
  freePointer(ztmp);
}

void
NekRSMesh::volumeVertices()
{
  // nekRS has already performed a global operation such that all processes know the
  // toal number of volume elements and their phase
  int n_vertices_in_mirror = _n_build_per_volume_elem * _n_volume_elems * _n_vertices_per_volume;
  double * x = (double *) malloc(n_vertices_in_mirror * sizeof(double));
  double * y = (double *) malloc(n_vertices_in_mirror * sizeof(double));
  double * z = (double *) malloc(n_vertices_in_mirror * sizeof(double));
  double * p = (double *) malloc(_n_build_per_volume_elem * _n_volume_elems * sizeof(double));

  nrs_t * nrs = (nrs_t *)nekrs::nrsPtr();
  int rank = nekrs::commRank();

  mesh_t * mesh;
  int Np_mirror;

  if (_order == 0)
  {
    // For a first-order mesh mirror, we can take a shortcut and instead just fetch the
    // corner nodes. In this case, 'mesh' is no longer a custom-build mesh copy, but the
    // actual mesh for computation
    mesh = _nek_internal_mesh;
    Np_mirror = 8;
  }
  else
  {
    // Create a duplicate of the solution mesh, but with the desired order of the mesh interpolation.
    // Then we can just read the coordinates of the GLL points to find the libMesh node positions.
    // We only need to do this if the mesh is not already N = 2.
    if (_nek_internal_mesh->N == 2)
      mesh = _nek_internal_mesh;
    else
      mesh = createMesh(platform->comm.mpiComm, _order + 1, 0, nrs->cht, *(nrs->kernelInfo));
    Np_mirror = mesh->Np;
  }

  // Allocate space for the coordinates and phase that are on this rank
  int n_vertices_on_rank = _n_build_per_volume_elem * _volume_coupling.n_elems * Np_mirror;
  double * xtmp = (double *) malloc(n_vertices_on_rank * sizeof(double));
  double * ytmp = (double *) malloc(n_vertices_on_rank * sizeof(double));
  double * ztmp = (double *) malloc(n_vertices_on_rank * sizeof(double));
  double * ptmp = (double *) malloc(_n_build_per_volume_elem * _volume_coupling.n_elems * sizeof(double));

  int c = 0;
  int d = 0;
  for (int k = 0; k < _volume_coupling.total_n_elems; ++k)
  {
    if (_volume_coupling.process[k] == rank)
    {
      int i = _volume_coupling.element[k];
      int offset = i * mesh->Np;

      for (int build = 0; build < _n_build_per_volume_elem; ++build)
      {
        ptmp[d++] = i >= nekrs::flowMesh()->Nelements;
        for (int v = 0; v < Np_mirror; ++v, ++c)
        {
          int vertex_offset = _order == 0 ? _corner_indices[build][v] : v;
          int id = offset + vertex_offset;

          xtmp[c] = mesh->x[id];
          ytmp[c] = mesh->y[id];
          ztmp[c] = mesh->z[id];
        }
      }
    }
  }

  nekrs::allgatherv(_volume_coupling.mirror_counts, xtmp, x, Np_mirror);
  nekrs::allgatherv(_volume_coupling.mirror_counts, ytmp, y, Np_mirror);
  nekrs::allgatherv(_volume_coupling.mirror_counts, ztmp, z, Np_mirror);
  nekrs::allgatherv(_volume_coupling.mirror_counts, ptmp, p);

  for (int i = 0; i < _n_build_per_volume_elem * _n_volume_elems; ++i)
    _phase.push_back(p[i]);

  for (int i = 0; i < n_vertices_in_mirror; ++i)
  {
    _x.push_back(x[i]);
    _y.push_back(y[i]);
    _z.push_back(z[i]);
  }

  freePointer(x);
  freePointer(y);
  freePointer(z);
  freePointer(p);
  freePointer(xtmp);
  freePointer(ytmp);
  freePointer(ztmp);
  freePointer(ptmp);
}

void
NekRSMesh::extractSurfaceMesh()
{
  // Find the global vertex IDs that are on the _boundary. Note that nekRS performs a
  // global communciation here such that each nekRS process has knowledge of all the
  // boundary information.
  faceVertices();

  _new_elem = &NekRSMesh::boundaryElem;
  _n_elems = _n_surface_elems;
  _n_vertices_per_elem = _n_vertices_per_surface;
  _n_moose_per_nek = _n_build_per_surface_elem;
  _node_index = &_bnd_node_index;
  _elem_processor_id = &NekRSMesh::boundaryElemProcessorID;
}

void
NekRSMesh::extractVolumeMesh()
{
  // Find the global vertex IDs in the volume. Note that nekRS performs a
  // global communciation here such that each nekRS process has knowledge of all the
  // volume information.
  volumeVertices();

  _new_elem = &NekRSMesh::volumeElem;
  _n_elems = _n_volume_elems;
  _n_vertices_per_elem = _n_vertices_per_volume;
  _n_moose_per_nek = _n_build_per_volume_elem;
  _node_index = &_vol_node_index;
  _elem_processor_id = &NekRSMesh::volumeElemProcessorID;
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

int
NekRSMesh::boundaryElemProcessorID(const int elem_id)
{
  return _boundary_coupling.processor_id(elem_id);
}

int
NekRSMesh::volumeElemProcessorID(const int elem_id)
{
  return _volume_coupling.processor_id(elem_id);
}

int
NekRSMesh::boundary_id(const int elem_id, const int face_id)
{
  return _volume_coupling.boundary[elem_id * _nek_internal_mesh->Nfaces + face_id];
}

int
NekRSMesh::facesOnBoundary(const int elem_id) const
{
  return _volume_coupling.n_faces_on_boundary[elem_id];
}

void
NekRSMesh::updateDisplacement(const int e, const double *src, const field::NekWriteEnum field)
{
  int nsrc = _volume? _n_vertices_per_volume : _n_vertices_per_surface;
  int offset = e * nsrc;

  switch (field)
  {
    case field::x_displacement:
      memcpy(&_prev_disp_x[offset], src, nsrc * sizeof(double));
      break;
    case field::y_displacement:
      memcpy(&_prev_disp_y[offset], src, nsrc * sizeof(double));
      break;
    case field::z_displacement:
      memcpy(&_prev_disp_z[offset], src, nsrc * sizeof(double));
      break;
    default:
      throw std::runtime_error("Unhandled NekWriteEnum in NekRSMesh::copyToDisplacement!\n");
  }
}
#endif
