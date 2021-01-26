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
  return nrs->Nscalar ? nrs->options.compareArgs("SCALAR00 IS TEMPERATURE", "TRUE") : false;
}

bool hasTemperatureSolve()
{
  nrs_t * nrs = (nrs_t *) nrsPtr();
  return hasTemperatureVariable() ? nrs->cds->compute[0] : false;
}

bool scratchAvailable()
{
  nrs_t * nrs = (nrs_t *) nrsPtr();

  // Because these scratch spaces are available for whatever the user sees fit, it is
  // possible that the user wants to use these arrays for a _different_ purpose aside from
  // transferring heat flux values. In nekrs::setup, we call the UDF_Setup0, UDF_Setup,
  // and UDF_ExecuteStep routines. These scratch space arrays aren't initialized anywhere
  // else in the core base, so we will make sure to throw an error from MOOSE if these
  // arrays are already in use, because otherwise our flux transfer might get overwritten
  // by whatever other operation the user is trying to do.
  if (nrs->usrwrk)
    return false;

  return true;
}

void initializeScratch()
{
  nrs_t * nrs = (nrs_t *) nrsPtr();
  mesh_t * mesh = nrs->cds->mesh;

  // clear them just to be sure
  if (nrs->usrwrk)
  {
    nrs->o_usrwrk.free();
    free(nrs->usrwrk);
  }

  // In order to make indexing simpler in the devide user functions (which is where the
  // boundary conditions are then actually applied), we define these scratch arrays
  // as volume arrays. This means we must take some care as to how we write into these
  // because our coupling is a surface-based coupling. Because cds->o_usrwrk points to the
  // same address as nrs->o_usrwrk, we can simply work with the arrays on nrs.
  nrs->usrwrk = (double *) calloc(mesh->Nelements * mesh->Np, sizeof(double));
  nrs->o_usrwrk = mesh->device.malloc(mesh->Nelements * mesh->Np * sizeof(double), nrs->usrwrk);
}

void freeScratch()
{
  nrs_t * nrs = (nrs_t *) nrsPtr();

  if (nrs->usrwrk)
  {
    free(nrs->usrwrk);
    nrs->o_usrwrk.free();
  }
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

void temperature(const double * I, const int order, const bool needs_interpolation, double* T)
{
  nrs_t * nrs = (nrs_t *) nrsPtr();
  mesh_t* mesh = nrs->cds->mesh;

  int start_1d = mesh->Nq;
  int end_1d = order + 2;
  int start_2d = start_1d * start_1d;
  int end_2d = end_1d * end_1d;

  // allocate temporary space to hold the results of the search for each process
  double* Ttmp = (double*) calloc(nek_boundary_coupling.n_faces * end_2d, sizeof(double));

  // initialize scratch space for the face temperature so that we can easily
  // pass in face-initialized values to interpolateSurfaceFaceHex3D
  double* Tface = (double*) calloc(start_2d, sizeof(double));

  // initialize scratch space for the interpolation process so that we don't need to
  // allocate and free it for every call to interpolateSurfaceFaceHex3D
  double* scratch = (double*) calloc(start_1d * end_1d, sizeof(double));

  // if we apply the shortcut for first-order interpolations, just hard-code those
  // indices that we'll grab for a surface hex element
  int indices [] = {0, start_1d - 1, start_2d - start_1d, start_2d - 1};

  int c = 0;
  for (int k = 0; k < nek_boundary_coupling.total_n_faces; ++k)
  {
    if (nek_boundary_coupling.process[k] == mesh->rank)
    {
      int i = nek_boundary_coupling.element[k];
      int j = nek_boundary_coupling.face[k];
      int offset = i * mesh->Nfaces * start_2d + j * start_2d;

      if (needs_interpolation)
      {
        // get the temperature on the face
        for (int v = 0; v < start_2d; ++v)
        {
          int id = mesh->vmapM[offset + v];
          Tface[v] = nrs->cds->S[id];
        }

        // and then interpolate it
        interpolateSurfaceFaceHex3D(scratch, I, Tface, start_1d, &(Ttmp[c]), end_1d);
        c += end_2d;
      }
      else
      {
        // get the temperature on the face. We assume on the MOOSE side that
        // we'll only try this shortcut if the mesh is first order (since the second
        // order case can only skip the interpolation if nekRS's polynomial order is
        // 2, which is unlikely for actual calculations.
        for (int v = 0; v < end_2d; ++v, ++c)
        {
          int id = mesh->vmapM[offset + indices[v]];
          Ttmp[c] = nrs->cds->S[id];
        }
      }
    }
  }

  int* recvCounts = (int *) calloc(mesh->size, sizeof(int));
  int* displacement = (int *) calloc(mesh->size, sizeof(int));
  displacementAndCounts(recvCounts, displacement, end_2d);

  MPI_Allgatherv(Ttmp, recvCounts[mesh->rank], MPI_DOUBLE, T,
    (const int*)recvCounts, (const int*)displacement, MPI_DOUBLE, mesh->comm);

  free(recvCounts);
  free(displacement);
  free(Ttmp);
  free(Tface);
  free(scratch);
}

int processor_id(const int elem_id)
{
  return nek_boundary_coupling.process[elem_id];
}

void flux(const double * I, const int elem_id, const int order, double * flux_face)
{
  nrs_t * nrs = (nrs_t *) nrsPtr();
  mesh_t * mesh = nrs->cds->mesh;

  int end_1d = mesh->Nq;
  int start_1d = order + 2;
  int end_2d = end_1d * end_1d;
  int start_2d = start_1d * start_1d;

  // We can only write into the nekRS scratch space if that face is "owned" by the current process
  if (mesh->rank == processor_id(elem_id))
  {
    int e = nek_boundary_coupling.element[elem_id];
    int f = nek_boundary_coupling.face[elem_id];

    double * scratch = (double*) calloc(start_1d * end_1d, sizeof(double));
    double * flux_tmp = (double*) calloc(end_2d, sizeof(double));

    interpolateSurfaceFaceHex3D(scratch, I, flux_face, start_1d, flux_tmp, end_1d);

    int offset = e * mesh->Nfaces * mesh->Nfp + f * mesh->Nfp;
    for (int i = 0; i < end_2d; ++i)
    {
      int id = mesh->vmapM[offset + i];
      nrs->usrwrk[id] = flux_tmp[i];
    }

    free(scratch);
    free(flux_tmp);
  }
}

double fluxIntegral()
{
  nrs_t * nrs = (nrs_t *) nrsPtr();
  mesh_t * mesh = nrs->cds->mesh;

  double integral = 0.0;

  for (int k = 0; k < nek_boundary_coupling.total_n_faces; ++k)
  {
    if (nek_boundary_coupling.process[k] == mesh->rank)
    {
      int i = nek_boundary_coupling.element[k];
      int j = nek_boundary_coupling.face[k];
      int offset = i * mesh->Nfaces * mesh->Nfp + j * mesh->Nfp;

      for (int v = 0; v < mesh->Nfp; ++v)
        integral += nrs->usrwrk[mesh->vmapM[offset + v]] * mesh->sgeo[mesh->Nsgeo * (offset + v) + WSJID];
    }
  }

  // sum across all processes
  double total_integral;
  MPI_Allreduce(&integral, &total_integral, 1, MPI_DOUBLE, MPI_SUM, mesh->comm);

  return total_integral;
}

void normalizeFlux(const double moose_integral, const double nek_integral)
{
  // avoid divide-by-zero
  if (std::abs(nek_integral) < abs_tol)
    return;

  nrs_t * nrs = (nrs_t *) nrsPtr();
  mesh_t * mesh = nrs->cds->mesh;

  const double ratio = moose_integral / nek_integral;

  for (int k = 0; k < nek_boundary_coupling.total_n_faces; ++k)
  {
    if (nek_boundary_coupling.process[k] == mesh->rank)
    {
      int i = nek_boundary_coupling.element[k];
      int j = nek_boundary_coupling.face[k];
      int offset = i * mesh->Nfaces * mesh->Nfp + j * mesh->Nfp;

      for (int v = 0; v < mesh->Nfp; ++v)
      {
        int id = mesh->vmapM[offset + v];
        nrs->usrwrk[id] *= ratio;
      }
    }
  }
}

void copyFluxToDevice()
{
  nrs_t * nrs = (nrs_t *) nrsPtr();
  nrs->o_usrwrk.copyFrom(nrs->usrwrk);
}

double sideMaxValue(const std::vector<int> & boundary_id, const field::NekFieldEnum & field)
{
  nrs_t * nrs = (nrs_t *) nrsPtr();
  mesh_t * mesh = nrs->cds->mesh;

  double value = -std::numeric_limits<double>::max();
  MPI_Op reduction_type;

  const double (*f) (int);

  // find the field for which we are finding the extreme value
  switch (field)
  {
    case field::temperature:
      f = &solution::temperature;
      break;
    default:
      throw std::runtime_error("Unhandled 'NekFieldEnum'!");
  }

  for (int i = 0; i < mesh->Nelements; ++i) {
    for (int j = 0; j < mesh->Nfaces; ++j) {
      int face_id = mesh->EToB[i * mesh->Nfaces + j];

      if (std::find(boundary_id.begin(), boundary_id.end(), face_id) != boundary_id.end())
      {
        int offset = i * mesh->Nfaces * mesh->Nfp + j * mesh->Nfp;
        for (int v = 0; v < mesh->Nfp; ++v)
          value = std::max(value, f(mesh->vmapM[offset + v]));
      }
    }
  }

  // find extreme value across all processes
  double reduced_value;
  MPI_Allreduce(&value, &reduced_value, 1, MPI_DOUBLE, MPI_MAX, mesh->comm);

  return reduced_value;
}

double volumeMaxValue(const field::NekFieldEnum & field)
{
  nrs_t * nrs = (nrs_t *) nrsPtr();
  mesh_t * mesh = nrs->cds->mesh;

  double value = -std::numeric_limits<double>::max();

  const double (*f) (int);

  // find the field for which we are finding the extreme value
  switch (field)
  {
    case field::temperature:
      f = &solution::temperature;
      break;
    default:
      throw std::runtime_error("Unhandled 'NekFieldEnum'!");
  }

  for (int i = 0; i < mesh->Nelements; ++i) {
    for (int j = 0; j < mesh->Np; ++j) {
      int id = i * mesh->Np + j;
      value = std::max(value, f(i * mesh->Np + j));
    }
  }

  // find extreme value across all processes
  double reduced_value;
  MPI_Allreduce(&value, &reduced_value, 1, MPI_DOUBLE, MPI_MAX, mesh->comm);

  return reduced_value;
}

double volumeMinValue(const field::NekFieldEnum & field)
{
  nrs_t * nrs = (nrs_t *) nrsPtr();
  mesh_t * mesh = nrs->cds->mesh;

  double value = std::numeric_limits<double>::max();

  const double (*f) (int);

  // find the field for which we are finding the extreme value
  switch (field)
  {
    case field::temperature:
      f = &solution::temperature;
      break;
    default:
      throw std::runtime_error("Unhandled 'NekFieldEnum'!");
  }

  for (int i = 0; i < mesh->Nelements; ++i) {
    for (int j = 0; j < mesh->Np; ++j) {
      int id = i * mesh->Np + j;
      value = std::min(value, f(i * mesh->Np + j));
    }
  }

  // find extreme value across all processes
  double reduced_value;
  MPI_Allreduce(&value, &reduced_value, 1, MPI_DOUBLE, MPI_MIN, mesh->comm);

  return reduced_value;
}

double sideMinValue(const std::vector<int> & boundary_id, const field::NekFieldEnum & field)
{
  nrs_t * nrs = (nrs_t *) nrsPtr();
  mesh_t * mesh = nrs->cds->mesh;

  double value = std::numeric_limits<double>::max();

  const double (*f) (int);

  // find the field for which we are finding the extreme value
  switch (field)
  {
    case field::temperature:
      f = &solution::temperature;
      break;
    default:
      throw std::runtime_error("Unhandled 'NekFieldEnum'!");
  }

  for (int i = 0; i < mesh->Nelements; ++i) {
    for (int j = 0; j < mesh->Nfaces; ++j) {
      int face_id = mesh->EToB[i * mesh->Nfaces + j];

      if (std::find(boundary_id.begin(), boundary_id.end(), face_id) != boundary_id.end())
      {
        int offset = i * mesh->Nfaces * mesh->Nfp + j * mesh->Nfp;
        for (int v = 0; v < mesh->Nfp; ++v) {
          value = std::min(value, f(mesh->vmapM[offset + v]));
        }
      }
    }
  }

  // find extreme value across all processes
  double reduced_value;
  MPI_Allreduce(&value, &reduced_value, 1, MPI_DOUBLE, MPI_MIN, mesh->comm);

  return reduced_value;
}

double sideIntegral(const std::vector<int> & boundary_id, const field::NekFieldEnum & integrand)
{
  nrs_t * nrs = (nrs_t *) nrsPtr();
  mesh_t * mesh = nrs->cds->mesh;

  double integral = 0.0;

  const double (*f) (int);

  switch (integrand)
  {
    case field::temperature:
      f = &solution::temperature;
      break;
    case field::unity:
      f = &solution::unity;
      break;
    default:
      throw std::runtime_error("Unhandled 'NekFieldEnum'!");
  }

  for (int i = 0; i < mesh->Nelements; ++i) {
    for (int j = 0; j < mesh->Nfaces; ++j) {
      int face_id = mesh->EToB[i * mesh->Nfaces + j];

      if (std::find(boundary_id.begin(), boundary_id.end(), face_id) != boundary_id.end())
      {
        int offset = i * mesh->Nfaces * mesh->Nfp + j * mesh->Nfp;
        for (int v = 0; v < mesh->Nfp; ++v) {
          integral += f(mesh->vmapM[offset + v]) * mesh->sgeo[mesh->Nsgeo * (offset + v) + WSJID];
        }
      }
    }
  }

  // sum across all processes
  double total_integral;
  MPI_Allreduce(&integral, &total_integral, 1, MPI_DOUBLE, MPI_SUM, mesh->comm);

  return total_integral;
}

double sideMassFluxWeightedIntegral(const std::vector<int> & boundary_id, const field::NekFieldEnum & integrand)
{
  nrs_t * nrs = (nrs_t *) nrsPtr();
  mesh_t * mesh = nrs->cds->mesh;

  // TODO: This function only works correctly if the density is constant, because
  // otherwise we need to copy the density from device to host
  double rho;
  nrs->options.getArgs("DENSITY", rho);

  double integral = 0.0;

  const double (*f) (int);

  switch (integrand)
  {
    case field::temperature:
      f = &solution::temperature;
      break;
    case field::unity:
      f = &solution::unity;
      break;
    default:
      throw std::runtime_error("Unhandled 'NekFieldEnum'!");
  }

  for (int i = 0; i < mesh->Nelements; ++i) {
    for (int j = 0; j < mesh->Nfaces; ++j) {
      int face_id = mesh->EToB[i * mesh->Nfaces + j];

      if (std::find(boundary_id.begin(), boundary_id.end(), face_id) != boundary_id.end())
      {
        int offset = i * mesh->Nfaces * mesh->Nfp + j * mesh->Nfp;
        for (int v = 0; v < mesh->Nfp; ++v) {
          int vol_id = mesh->vmapM[offset + v];
          int surf_offset = mesh->Nsgeo * (offset + v);

          double normal_velocity =
            nrs->U[vol_id + 0 * nrs->fieldOffset] * mesh->sgeo[surf_offset + NXID] +
            nrs->U[vol_id + 1 * nrs->fieldOffset] * mesh->sgeo[surf_offset + NYID] +
            nrs->U[vol_id + 2 * nrs->fieldOffset] * mesh->sgeo[surf_offset + NZID];

          integral += f(vol_id) * rho * normal_velocity * mesh->sgeo[surf_offset + WSJID];
        }
      }
    }
  }

  // sum across all processes
  double total_integral;
  MPI_Allreduce(&integral, &total_integral, 1, MPI_DOUBLE, MPI_SUM, mesh->comm);

  return total_integral;
}

double heatFluxIntegral(const std::vector<int> & boundary_id)
{
  nrs_t * nrs = (nrs_t *) nrsPtr();
  mesh_t * mesh = nrs->cds->mesh;

  // TODO: This function only works correctly if the conductivity is constant, because
  // otherwise we need to copy the density from device to host
  double k;
  nrs->options.getArgs("SCALAR00 DIFFUSIVITY", k);

  double integral = 0.0;

  double * grad_T = (double *) calloc(3 * nrs->cds->fieldOffset, sizeof(double));
  gradient(nrs->cds->fieldOffset, nrs->cds->S, grad_T);

  for (int i = 0; i < mesh->Nelements; ++i) {
    for (int j = 0; j < mesh->Nfaces; ++j) {
      int face_id = mesh->EToB[i * mesh->Nfaces + j];

      if (std::find(boundary_id.begin(), boundary_id.end(), face_id) != boundary_id.end())
      {
        int offset = i * mesh->Nfaces * mesh->Nfp + j * mesh->Nfp;
        for (int v = 0; v < mesh->Nfp; ++v) {
          int vol_id = mesh->vmapM[offset + v];
          int surf_offset = mesh->Nsgeo * (offset + v);

          double normal_grad_T =
            grad_T[vol_id + 0 * nrs->cds->fieldOffset] * mesh->sgeo[surf_offset + NXID] +
            grad_T[vol_id + 1 * nrs->cds->fieldOffset] * mesh->sgeo[surf_offset + NYID] +
            grad_T[vol_id + 2 * nrs->cds->fieldOffset] * mesh->sgeo[surf_offset + NZID];

          integral += -k * normal_grad_T * mesh->sgeo[surf_offset + WSJID];
        }
      }
    }
  }

  free(grad_T);

  // sum across all processes
  double total_integral;
  MPI_Allreduce(&integral, &total_integral, 1, MPI_DOUBLE, MPI_SUM, mesh->comm);

  return total_integral;
}


void gradient(const int offset, const double * f, double * grad_f)
{
  nrs_t * nrs = (nrs_t *) nrsPtr();
  mesh_t * mesh = nrs->cds->mesh;

  std::vector<std::vector<std::vector<double>>> s_P(mesh->Nq, std::vector<std::vector<double>>(mesh->Nq, std::vector<double>(mesh->Nq, 0)));
  std::vector<std::vector<double>> s_D(mesh->Nq, std::vector<double>(mesh->Nq, 0));

  for (int e = 0; e < mesh->Nelements; ++e)
  {
    for (int k = 0; k < mesh->Nq; ++k)
      for (int j = 0; j < mesh->Nq; ++j)
        for (int i = 0; i < mesh->Nq; ++i) {
          const int id = e * mesh->Np + k * mesh->Nq * mesh->Nq + j * mesh->Nq + i;

          s_P[k][j][i]  = f[id];

          if (k == 0)
            s_D[j][i] = mesh->D[j * mesh->Nq + i];
        }

    for (int k = 0; k < mesh->Nq; ++k) {
      for (int j = 0; j < mesh->Nq; ++j) {
        for (int i = 0; i < mesh->Nq; ++i) {
          const int gid = e * mesh->Np * mesh->Nvgeo + k * mesh->Nq * mesh->Nq + j * mesh->Nq + i;
          const double drdx = mesh->vgeo[gid + RXID * mesh->Np];
          const double drdy = mesh->vgeo[gid + RYID * mesh->Np];
          const double drdz = mesh->vgeo[gid + RZID * mesh->Np];
          const double dsdx = mesh->vgeo[gid + SXID * mesh->Np];
          const double dsdy = mesh->vgeo[gid + SYID * mesh->Np];
          const double dsdz = mesh->vgeo[gid + SZID * mesh->Np];
          const double dtdx = mesh->vgeo[gid + TXID * mesh->Np];
          const double dtdy = mesh->vgeo[gid + TYID * mesh->Np];
          const double dtdz = mesh->vgeo[gid + TZID * mesh->Np];

          // compute 'r' and 's' derivatives of (q_m) at node n
          double dpdr  = 0.f, dpds  = 0.f, dpdt  = 0.f;

          for(int n = 0; n < mesh->Nq; ++n)
          {
            const double Dr = s_D[i][n];
            const double Ds = s_D[j][n];
            const double Dt = s_D[k][n];

            dpdr += Dr * s_P[k][j][n];
            dpds += Ds * s_P[k][n][i];
            dpdt += Dt * s_P[n][j][i];
          }

          const int id = e * mesh->Np + k * mesh->Nq * mesh->Nq + j * mesh->Nq + i;
          grad_f[id + 0 * offset] = drdx * dpdr + dsdx * dpds + dtdx * dpdt;
          grad_f[id + 1 * offset] = drdy * dpdr + dsdy * dpds + dtdy * dpdt;
          grad_f[id + 2 * offset] = drdz * dpdr + dsdz * dpds + dtdz * dpdt;
        }
      }
    }
  }
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

namespace solution
{
  const double temperature(const int id)
  {
    nrs_t * nrs = (nrs_t *) nrsPtr();
    return nrs->cds->S[id];
  }

  const double unity(const int /* id */)
  {
    return 1.0;
  }
} // end namespace solution

} // end namespace nekrs
