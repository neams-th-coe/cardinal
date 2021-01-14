#include "NekInterface.h"
#include "nekrs.cpp"
#include "bcMap.hpp"

static nekrs::mesh::boundaryCoupling nek_boundary_coupling;

namespace nekrs
{

// various constants for controlling tolerances
constexpr double abs_tol = 1e-14;
constexpr double rel_tol = 1e-5;

bool endControlElapsedTime()
{
  nrs_t * nrs = (nrs_t *) nrsPtr();
  return !nrs->options.getArgs("STOP AT ELAPSED TIME").empty();
}

bool endControlTime()
{
  return endTime() > 0;
}

bool endControlNumSteps()
{
  return !endControlElapsedTime() && !endControlTime();
}

bool hasTemperatureVariable()
{
  nrs_t * nrs = (nrs_t *) nrsPtr();
  return nrs->options.compareArgs("TEMPERATURE", "TRUE");
}

bool hasTemperatureSolve()
{
  nrs_t * nrs = (nrs_t *) nrsPtr();
  return hasTemperatureVariable() ? nrs->cds->compute[0] : false;
}

void interpolationMatrix(double * I, int starting_points, int ending_points)
{
  DegreeRaiseMatrix1D(starting_points - 1, ending_points - 1, I);
}

void interpolateSurfaceFaceHex3D(double* scratch, const double* I, double* x, int N, double* Ix, int M)
{
  for(int j = 0; j < N; ++j)
    for(int i = 0; i < M; ++i) {
      double tmp = 0;
      for(int n = 0; n < N; ++n) {
        tmp += I[i * N + n] * x[j * N + n];
        }
      scratch[j * M + i] = tmp;
    }

  for(int j = 0; j < M; ++j)
    for(int i = 0; i < M; ++i) {
      double tmp = 0;
      for(int n = 0; n < N; ++n) {
        tmp += I[j * N + n] * scratch[n * M + i];
      }
      Ix[j * M + i] = tmp;
    }
}

void displacementAndCounts(int * counts, int * displacement, const int multiplier = 1.0)
{
  nrs_t * nrs = (nrs_t *) nrsPtr();
  mesh_t * mesh = nrs->cds->mesh;

  for (int i = 0; i < mesh->size; ++i)
    counts[i] = nek_boundary_coupling.counts[i] * multiplier;

  displacement[0] = 0;
  for(int i = 1; i < mesh->size; i++)
    displacement[i] = displacement[i - 1] + counts[i - 1];
}

int processor_id(const int elem_id)
{
  return nek_boundary_coupling.process[elem_id];
}

namespace mesh
{

bool isHeatFluxBoundary(const int boundary)
{
  return bcMap::text(boundary, "scalar00") == "fixedGradient";
}

const std::string temperatureBoundaryType(const int boundary)
{
  return bcMap::text(boundary, "scalar00");
}

int polynomialOrder()
{
  nrs_t * nrs = (nrs_t *) nrsPtr();
  return nrs->cds->mesh->N;
}

int Nelements()
{
  nrs_t * nrs = (nrs_t *) nrsPtr();
  int n_local = nrs->cds->mesh->Nelements;
  int n_global;
  MPI_Allreduce(&n_local, &n_global, 1, MPI_INT, MPI_SUM, nrs->cds->mesh->comm);
  return n_global;
}

int dim()
{
  nrs_t * nrs = (nrs_t *) nrsPtr();
  return nrs->cds->mesh->dim;
}

int NfaceVertices()
{
  nrs_t * nrs = (nrs_t *) nrsPtr();
  return nrs->cds->mesh->NfaceVertices;
}

int NboundaryFaces()
{
  nrs_t * nrs = (nrs_t *) nrsPtr();
  return nrs->cds->mesh->NboundaryFaces;
}

int NboundaryID()
{
  nrs_t * nrs = (nrs_t *) nrsPtr();

  if (nrs->mesh->cht)
    return nekData.NboundaryIDt;
  else
    return nekData.NboundaryID;
}

bool validBoundaryIDs(const std::vector<int> & boundary_id, int & first_invalid_id, int & n_boundaries)
{
  n_boundaries = NboundaryID();

  bool valid_boundary_ids = true;
  for (const auto & b : boundary_id)
  {
    if ((b > n_boundaries) || (b <= 0))
    {
      first_invalid_id = b;
      valid_boundary_ids = false;
    }
  }

  return valid_boundary_ids;
}

void faceVertices(const std::vector<int> & boundary_id, const int order, float* x, float* y, float* z, int& N)
{
  nrs_t * nrs = (nrs_t *) nrsPtr();

  // Create a duplicate of the solution mesh, but with the desired order of the mesh interpolation.
  // Then we can just read the coordinates of the GLL points to find the libMesh node positions.
  mesh_t * mesh = createMesh(nrs->cds->mesh->comm, order + 1, 1 /* dummy, not used by 'faceVertices' */,
    nrs->cht, nrs->options, nrs->mesh->device, *(nrs->kernelInfo));

  // Because we won't know how many surface faces are owned by each process, we
  // allocate temporary space to hold the results of the search for each process. This
  // scratch space is sized to handle the maximum number of surface points, which would occur
  // if every nekRS surface is to be coupled to MOOSE
  int max_possible_surface_points = mesh->NboundaryFaces * mesh->Nfp;
  float* xtmp = (float*) malloc(max_possible_surface_points * sizeof(float));
  float* ytmp = (float*) malloc(max_possible_surface_points * sizeof(float));
  float* ztmp = (float*) malloc(max_possible_surface_points * sizeof(float));

  // Save information regarding the surface mesh coupling in terms of the process-local
  // element IDs, the element-local face IDs, and the process ownership.
  int max_possible_surfaces = mesh->NboundaryFaces;
  int* etmp = (int *) malloc(max_possible_surfaces * sizeof(int));
  int* ftmp = (int *) malloc(max_possible_surfaces * sizeof(int));
  int* ptmp = (int *) malloc(max_possible_surfaces * sizeof(int));

  nek_boundary_coupling.element = (int *) malloc(max_possible_surfaces * sizeof(int));
  nek_boundary_coupling.face = (int *) malloc(max_possible_surfaces * sizeof(int));
  nek_boundary_coupling.process = (int *) malloc(max_possible_surfaces * sizeof(int));

  // number of faces on boundary of interest for this process
  int Nfaces = 0;

  int c = 0;
  int d = 0;
  for (int i = 0; i < mesh->Nelements; ++i) {
    for (int j = 0; j < mesh->Nfaces; ++j) {
      int face_id = mesh->EToB[i * mesh->Nfaces + j];

      if (std::find(boundary_id.begin(), boundary_id.end(), face_id) != boundary_id.end())
      {
        Nfaces += 1;

        int offset = i * mesh->Nfaces * mesh->Nfp + j * mesh->Nfp;

        etmp[d] = i;
        ftmp[d] = j;
        ptmp[d] = mesh->rank;
        d++;

        for (int v = 0; v < mesh->Nfp; ++v, ++c) {
          int id = mesh->vmapM[offset + v];
          xtmp[c] = mesh->x[id];
          ytmp[c] = mesh->y[id];
          ztmp[c] = mesh->z[id];
        }
      }
    }
  }

  // gather all the boundary face counters and make available in N
  MPI_Allreduce(&Nfaces, &N, 1, MPI_INT, MPI_SUM, mesh->comm);
  nek_boundary_coupling.n_faces = Nfaces;
  nek_boundary_coupling.total_n_faces = N;

  // make available to all processes the number of faces owned by each process
  nek_boundary_coupling.counts = (int *) calloc(mesh->size, sizeof(int));
  MPI_Allgather(&Nfaces, 1, MPI_INT, nek_boundary_coupling.counts, 1, MPI_INT, mesh->comm);

  // compute the counts and displacement based on the GLL points
  int* recvCounts = (int *) calloc(mesh->size, sizeof(int));
  int* displacement = (int *) calloc(mesh->size, sizeof(int));
  displacementAndCounts(recvCounts, displacement, mesh->Nfp);

  MPI_Allgatherv(xtmp, recvCounts[mesh->rank], MPI_FLOAT, x,
    (const int*)recvCounts, (const int*)displacement, MPI_FLOAT, mesh->comm);

  MPI_Allgatherv(ytmp, recvCounts[mesh->rank], MPI_FLOAT, y,
    (const int*)recvCounts, (const int*)displacement, MPI_FLOAT, mesh->comm);

  MPI_Allgatherv(ztmp, recvCounts[mesh->rank], MPI_FLOAT, z,
    (const int*)recvCounts, (const int*)displacement, MPI_FLOAT, mesh->comm);

  // adjust the MPI allgather info for face-based data exchange
  displacementAndCounts(recvCounts, displacement);

  MPI_Allgatherv(etmp, recvCounts[mesh->rank], MPI_INT, nek_boundary_coupling.element,
    (const int*)recvCounts, (const int*)displacement, MPI_INT, mesh->comm);

  MPI_Allgatherv(ftmp, recvCounts[mesh->rank], MPI_INT, nek_boundary_coupling.face,
    (const int*)recvCounts, (const int*)displacement, MPI_INT, mesh->comm);

  MPI_Allgatherv(ptmp, recvCounts[mesh->rank], MPI_INT, nek_boundary_coupling.process,
    (const int*)recvCounts, (const int*)displacement, MPI_INT, mesh->comm);

  free(recvCounts);
  free(displacement);
  free(xtmp);
  free(ytmp);
  free(ztmp);
  free(etmp);
  free(ftmp);
  free(ptmp);
}

void freeMesh()
{
  if (nek_boundary_coupling.element) free(nek_boundary_coupling.element);
  if (nek_boundary_coupling.face) free(nek_boundary_coupling.face);
  if (nek_boundary_coupling.process) free(nek_boundary_coupling.process);
  if (nek_boundary_coupling.counts) free(nek_boundary_coupling.counts);
}

} // end namespace mesh

} // end namespace nekrs
