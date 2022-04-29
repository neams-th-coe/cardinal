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

#include "NekInterface.h"
#include "CardinalUtils.h"

static nekrs::solution::characteristicScales scales;

// Maximum number of fields that we pre-allocate in the scratch space array.
// The first five are *always* reserved for Cardinal's use to be used in nekRS
// - all others are still free for use for
// things like computing wall distances, etc. If anyone ever wants more than
// 7, we can just change this value and commit that to the Cardinal repo - this
// is just a number not reflective of any limitations anywhere, so we're free to
// make it bigger later if we need it. Seven just seems fine for now.
#define MAX_SCRATCH_FIELDS 7

namespace nekrs
{

// various constants for controlling tolerances
constexpr double abs_tol = 1e-8;
constexpr double rel_tol = 1e-5;

std::vector<int>
cornerGLLIndices(const int & n)
{
  int back_corner = (n + 1) * (n + 1) * n;
  std::vector<int> corner_indices = {0,
                                     n,
                                     (n + 1) * n,
                                     (n + 1) * (n + 1) - 1,
                                     back_corner,
                                     back_corner + n,
                                     back_corner + (n + 1) * n,
                                     back_corner + (n + 1) * (n + 1) - 1};
  return corner_indices;
}

void
write_field_file(const std::string & prefix, const dfloat time)
{
  nrs_t * nrs = (nrs_t *)nrsPtr();

  int Nscalar = 0;
  occa::memory o_s;
  if (nrs->Nscalar)
  {
    o_s = nrs->cds->o_S;
    Nscalar = nrs->Nscalar;
  }

  writeFld(prefix.c_str(), time, 1 /* coords */, 1 /* FP64 */, &nrs->o_U, &nrs->o_P, &o_s, Nscalar);
}

void
buildOnly(int buildOnly)
{
  build_only = buildOnly;
}

int
buildOnly()
{
  return build_only;
}

bool
hasCHT()
{
  return entireMesh()->cht;
}

bool
hasMovingMesh()
{
  return platform->options.compareArgs("MOVING MESH", "TRUE");
}

bool
hasVariableDt()
{
  return platform->options.compareArgs("VARIABLE DT", "TRUE");
}

bool
endControlElapsedTime()
{
  return !platform->options.getArgs("STOP AT ELAPSED TIME").empty();
}

bool
endControlTime()
{
  return endTime() > 0;
}

bool
endControlNumSteps()
{
  return !endControlElapsedTime() && !endControlTime();
}

bool
hasTemperatureVariable()
{
  nrs_t * nrs = (nrs_t *)nrsPtr();
  return nrs->Nscalar ? platform->options.compareArgs("SCALAR00 IS TEMPERATURE", "TRUE") : false;
}

bool
hasTemperatureSolve()
{
  nrs_t * nrs = (nrs_t *)nrsPtr();
  return hasTemperatureVariable() ? nrs->cds->compute[0] : false;
}

bool
hasScalarVariable(int scalarId)
{
  nrs_t * nrs = (nrs_t *)nrsPtr();
  return (scalarId < nrs->Nscalar);
}

bool
hasHeatSourceKernel()
{
  return udf.sEqnSource;
}

bool
isInitialized()
{
  nrs_t * nrs = (nrs_t *)nrsPtr();
  return nrs;
}

int
scalarFieldOffset()
{
  nrs_t * nrs = (nrs_t *)nrsPtr();
  return nrs->cds->fieldOffset[0];
}

int
velocityFieldOffset()
{
  nrs_t * nrs = (nrs_t *)nrsPtr();
  return nrs->fieldOffset;
}

mesh_t *
entireMesh()
{
  if (hasTemperatureVariable())
    return temperatureMesh();
  else
    return flowMesh();
}

mesh_t *
flowMesh()
{
  nrs_t * nrs = (nrs_t *)nrsPtr();
  return nrs->meshV;
}

mesh_t *
temperatureMesh()
{
  nrs_t * nrs = (nrs_t *)nrsPtr();
  return nrs->cds->mesh[0];
}

int
commRank()
{
  return platform->comm.mpiRank;
}

int
commSize()
{
  return platform->comm.mpiCommSize;
}

bool
scratchAvailable()
{
  nrs_t * nrs = (nrs_t *)nrsPtr();

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

void
initializeScratch()
{
  nrs_t * nrs = (nrs_t *)nrsPtr();
  mesh_t * mesh = temperatureMesh();

  // clear them just to be sure
  freeScratch();

  // In order to make indexing simpler in the device user functions (which is where the
  // boundary conditions are then actually applied), we define these scratch arrays
  // as volume arrays. At the point that this function is called, we don't know if we have
  // boundary coupling, volume coupling, or both. So, we allocate enough space here to hold
  // multiple data transfers. These fields are always stored in this
  // order - i.e. if we only had volume couping, we would only start writing to this array
  // beginning at index nrs->cds->fieldOffset.
  nrs->usrwrk = (double *)calloc(MAX_SCRATCH_FIELDS * scalarFieldOffset(), sizeof(double));
  nrs->o_usrwrk = platform->device.malloc(MAX_SCRATCH_FIELDS * scalarFieldOffset() * sizeof(double),
                                          nrs->usrwrk);
}

void
freeScratch()
{
  nrs_t * nrs = (nrs_t *)nrsPtr();

  freePointer(nrs->usrwrk);
  nrs->o_usrwrk.free();
}

double
viscosity()
{
  dfloat mu;
  setupAide & options = platform->options;
  options.getArgs("VISCOSITY", mu);

  // because we set rho_ref, U_ref, and L_ref all equal to 1 if our input is dimensional,
  // we don't need to have separate treatments for dimensional vs. nondimensional cases
  dfloat Re = 1.0 / mu;
  return scales.rho_ref * scales.U_ref * scales.L_ref / Re;
}

double
Pr()
{
  dfloat rho, rho_cp, k;
  setupAide & options = platform->options;
  options.getArgs("DENSITY", rho);
  options.getArgs("SCALAR00 DENSITY", rho_cp);
  options.getArgs("SCALAR00 DIFFUSIVITY", k);

  dfloat Pe = 1.0 / k;
  dfloat conductivity = scales.rho_ref * scales.U_ref * scales.Cp_ref * scales.L_ref / Pe;
  dfloat Cp = rho_cp / rho * scales.Cp_ref;
  return viscosity() * Cp / conductivity;
}

void
interpolationMatrix(double * I, int starting_points, int ending_points)
{
  DegreeRaiseMatrix1D(starting_points - 1, ending_points - 1, I);
}

void
interpolateVolumeHex3D(const double * I, double * x, int N, double * Ix, int M)
{
  double * Ix1 = (dfloat *)calloc(N * N * M, sizeof(double));
  double * Ix2 = (dfloat *)calloc(N * M * M, sizeof(double));

  for (int k = 0; k < N; ++k)
    for (int j = 0; j < N; ++j)
      for (int i = 0; i < M; ++i)
      {
        dfloat tmp = 0;
        for (int n = 0; n < N; ++n)
          tmp += I[i * N + n] * x[k * N * N + j * N + n];
        Ix1[k * N * M + j * M + i] = tmp;
      }

  for (int k = 0; k < N; ++k)
    for (int j = 0; j < M; ++j)
      for (int i = 0; i < M; ++i)
      {
        dfloat tmp = 0;
        for (int n = 0; n < N; ++n)
          tmp += I[j * N + n] * Ix1[k * N * M + n * M + i];
        Ix2[k * M * M + j * M + i] = tmp;
      }

  for (int k = 0; k < M; ++k)
    for (int j = 0; j < M; ++j)
      for (int i = 0; i < M; ++i)
      {
        dfloat tmp = 0;
        for (int n = 0; n < N; ++n)
          tmp += I[k * N + n] * Ix2[n * M * M + j * M + i];
        Ix[k * M * M + j * M + i] = tmp;
      }

  freePointer(Ix1);
  freePointer(Ix2);
}

void
interpolateSurfaceFaceHex3D(
    double * scratch, const double * I, double * x, int N, double * Ix, int M)
{
  for (int j = 0; j < N; ++j)
    for (int i = 0; i < M; ++i)
    {
      double tmp = 0;
      for (int n = 0; n < N; ++n)
      {
        tmp += I[i * N + n] * x[j * N + n];
      }
      scratch[j * M + i] = tmp;
    }

  for (int j = 0; j < M; ++j)
    for (int i = 0; i < M; ++i)
    {
      double tmp = 0;
      for (int n = 0; n < N; ++n)
      {
        tmp += I[j * N + n] * scratch[n * M + i];
      }
      Ix[j * M + i] = tmp;
    }
}

void
displacementAndCounts(const std::vector<int> & base_counts,
                      int * counts,
                      int * displacement,
                      const int multiplier = 1.0)
{
  for (int i = 0; i < commSize(); ++i)
    counts[i] = base_counts[i] * multiplier;

  displacement[0] = 0;
  for (int i = 1; i < commSize(); i++)
    displacement[i] = displacement[i - 1] + counts[i - 1];
}

double
sourceIntegral(const NekVolumeCoupling & nek_volume_coupling)
{
  nrs_t * nrs = (nrs_t *)nrsPtr();
  mesh_t * mesh = temperatureMesh();

  double integral = 0.0;

  for (int k = 0; k < nek_volume_coupling.total_n_elems; ++k)
  {
    if (nek_volume_coupling.process[k] == commRank())
    {
      int i = nek_volume_coupling.element[k];
      int offset = i * mesh->Np;

      for (int v = 0; v < mesh->Np; ++v)
        integral += nrs->usrwrk[scalarFieldOffset() + offset + v] *
                    mesh->vgeo[mesh->Nvgeo * offset + v + mesh->Np * JWID];
    }
  }

  // sum across all processes
  double total_integral;
  MPI_Allreduce(&integral, &total_integral, 1, MPI_DOUBLE, MPI_SUM, platform->comm.mpiComm);

  return total_integral;
}

double
fluxIntegral(const NekBoundaryCoupling & nek_boundary_coupling)
{
  nrs_t * nrs = (nrs_t *)nrsPtr();
  mesh_t * mesh = temperatureMesh();

  double integral = 0.0;

  for (int k = 0; k < nek_boundary_coupling.total_n_faces; ++k)
  {
    if (nek_boundary_coupling.process[k] == commRank())
    {
      int i = nek_boundary_coupling.element[k];
      int j = nek_boundary_coupling.face[k];
      int offset = i * mesh->Nfaces * mesh->Nfp + j * mesh->Nfp;

      for (int v = 0; v < mesh->Nfp; ++v)
        integral +=
            nrs->usrwrk[mesh->vmapM[offset + v]] * mesh->sgeo[mesh->Nsgeo * (offset + v) + WSJID];
    }
  }

  // sum across all processes
  double total_integral;
  MPI_Allreduce(&integral, &total_integral, 1, MPI_DOUBLE, MPI_SUM, platform->comm.mpiComm);

  return total_integral;
}

bool
normalizeFlux(const NekBoundaryCoupling & nek_boundary_coupling,
              const double moose_integral,
              double nek_integral,
              double & normalized_nek_integral)
{
  // scale the nek flux to dimensional form for the sake of normalizing against
  // a dimensional MOOSE flux
  nek_integral *= scales.A_ref * scales.flux_ref;

  // avoid divide-by-zero
  if (std::abs(nek_integral) < abs_tol)
    return true;

  nrs_t * nrs = (nrs_t *)nrsPtr();
  mesh_t * mesh = temperatureMesh();

  const double ratio = moose_integral / nek_integral;

  for (int k = 0; k < nek_boundary_coupling.total_n_faces; ++k)
  {
    if (nek_boundary_coupling.process[k] == commRank())
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

  // check that the normalization worked properly - confirm against dimensional form
  normalized_nek_integral = fluxIntegral(nek_boundary_coupling) * scales.A_ref * scales.flux_ref;
  bool low_rel_err = std::abs(normalized_nek_integral - moose_integral) / moose_integral < rel_tol;
  bool low_abs_err = std::abs(normalized_nek_integral - moose_integral) < abs_tol;

  return low_rel_err && low_abs_err;
}

bool
normalizeHeatSource(const NekVolumeCoupling & nek_volume_coupling,
                    const double moose_integral,
                    double nek_integral,
                    double & normalized_nek_integral)
{
  // scale the nek source to dimensional form for the sake of normalizing against
  // a dimensional MOOSE source
  nek_integral *= scales.V_ref * scales.source_ref;

  // avoid divide-by-zero
  if (std::abs(nek_integral) < abs_tol)
    return true;

  nrs_t * nrs = (nrs_t *)nrsPtr();
  mesh_t * mesh = temperatureMesh();

  const double ratio = moose_integral / nek_integral;

  for (int k = 0; k < nek_volume_coupling.total_n_elems; ++k)
  {
    if (nek_volume_coupling.process[k] == commRank())
    {
      int i = nek_volume_coupling.element[k];
      int id = i * mesh->Np;

      for (int v = 0; v < mesh->Np; ++v)
        nrs->usrwrk[scalarFieldOffset() + id + v] *= ratio;
    }
  }

  // check that the normalization worked properly
  normalized_nek_integral = sourceIntegral(nek_volume_coupling) * scales.V_ref * scales.source_ref;
  bool low_rel_err = std::abs(normalized_nek_integral - moose_integral) / moose_integral < rel_tol;
  bool low_abs_err = std::abs(normalized_nek_integral - moose_integral) < abs_tol;

  return low_rel_err && low_abs_err;
}

void
limitTemperature(const double * min_T, const double * max_T)
{
  // if no limiters are provided, simply return
  if (!min_T && !max_T)
    return;

  double minimum = min_T ? *min_T : std::numeric_limits<double>::min();
  double maximum = max_T ? *max_T : std::numeric_limits<double>::max();

  // nondimensionalize if necessary
  minimum = (minimum - scales.T_ref) / scales.dT_ref;
  maximum = (maximum - scales.T_ref) / scales.dT_ref;

  nrs_t * nrs = (nrs_t *)nrsPtr();
  mesh_t * mesh = temperatureMesh();

  for (int i = 0; i < mesh->Nelements; ++i)
  {
    for (int j = 0; j < mesh->Np; ++j)
    {
      int id = i * mesh->Np + j;

      if (nrs->cds->S[id] < minimum)
        nrs->cds->S[id] = minimum;
      if (nrs->cds->S[id] > maximum)
        nrs->cds->S[id] = maximum;
    }
  }

  // when complete, copy to device
  nrs->cds->o_S.copyFrom(nrs->cds->S);
}

void
copyScratchToDevice()
{
  nrs_t * nrs = (nrs_t *)nrsPtr();

  // From Cardinal, we only write to the first five "slices" in nrs->usrwrk. But, the user might
  // be writing other parts of this scratch space from the .udf file. So, we need to be sure
  // to only copy the slices reserved for Cardinal, so that we don't accidentally overwrite other
  // parts of o_usrwrk (which from the order of the UDF calls, would always happen *after* the
  // transfers into NekRS)

  // The type of data contained in each slice of nrs->usrwrk depends on the Cardinal Problem
  // class used, but we always reserve the first five slices for Cardinal coupling data.
  nrs->o_usrwrk.copyFrom(nrs->usrwrk, 5 * scalarFieldOffset() * sizeof(dfloat), 0);
}

void
copyDeformationToDevice()
{
  mesh_t * mesh = entireMesh();
  mesh->o_x.copyFrom(mesh->x);
  mesh->o_y.copyFrom(mesh->y);
  mesh->o_z.copyFrom(mesh->z);
  mesh->update();
  // update host geometric and volume factors from device in case of mesh deformation
  mesh->o_sgeo.copyTo(mesh->sgeo);
  mesh->o_vgeo.copyTo(mesh->vgeo);
}

double
sideMaxValue(const std::vector<int> & boundary_id, const field::NekFieldEnum & field)
{
  mesh_t * mesh = entireMesh();

  double value = -std::numeric_limits<double>::max();

  double (*f)(int);
  f = solution::solutionPointer(field);

  for (int i = 0; i < mesh->Nelements; ++i)
  {
    for (int j = 0; j < mesh->Nfaces; ++j)
    {
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
  MPI_Allreduce(&value, &reduced_value, 1, MPI_DOUBLE, MPI_MAX, platform->comm.mpiComm);

  // dimensionalize the field if needed
  solution::dimensionalize(field, reduced_value);

  // if temperature, we need to add the reference temperature
  if (field == field::temperature)
    reduced_value += scales.T_ref;

  return reduced_value;
}

double
volumeMaxValue(const field::NekFieldEnum & field)
{
  mesh_t * mesh = entireMesh();

  double value = -std::numeric_limits<double>::max();

  double (*f)(int);
  f = solution::solutionPointer(field);

  for (int i = 0; i < mesh->Nelements; ++i)
  {
    for (int j = 0; j < mesh->Np; ++j)
    {
      int id = i * mesh->Np + j;
      value = std::max(value, f(id));
    }
  }

  // find extreme value across all processes
  double reduced_value;
  MPI_Allreduce(&value, &reduced_value, 1, MPI_DOUBLE, MPI_MAX, platform->comm.mpiComm);

  // dimensionalize the field if needed
  solution::dimensionalize(field, reduced_value);

  // if temperature, we need to add the reference temperature
  if (field == field::temperature)
    reduced_value += scales.T_ref;

  return reduced_value;
}

double
volumeMinValue(const field::NekFieldEnum & field)
{
  mesh_t * mesh = entireMesh();

  double value = std::numeric_limits<double>::max();

  double (*f)(int);
  f = solution::solutionPointer(field);

  for (int i = 0; i < mesh->Nelements; ++i)
  {
    for (int j = 0; j < mesh->Np; ++j)
    {
      int id = i * mesh->Np + j;
      value = std::min(value, f(id));
    }
  }

  // find extreme value across all processes
  double reduced_value;
  MPI_Allreduce(&value, &reduced_value, 1, MPI_DOUBLE, MPI_MIN, platform->comm.mpiComm);

  // dimensionalize the field if needed
  solution::dimensionalize(field, reduced_value);

  // if temperature, we need to add the reference temperature
  if (field == field::temperature)
    reduced_value += scales.T_ref;

  return reduced_value;
}

double
sideMinValue(const std::vector<int> & boundary_id, const field::NekFieldEnum & field)
{
  mesh_t * mesh = entireMesh();

  double value = std::numeric_limits<double>::max();

  double (*f)(int);
  f = solution::solutionPointer(field);

  for (int i = 0; i < mesh->Nelements; ++i)
  {
    for (int j = 0; j < mesh->Nfaces; ++j)
    {
      int face_id = mesh->EToB[i * mesh->Nfaces + j];

      if (std::find(boundary_id.begin(), boundary_id.end(), face_id) != boundary_id.end())
      {
        int offset = i * mesh->Nfaces * mesh->Nfp + j * mesh->Nfp;
        for (int v = 0; v < mesh->Nfp; ++v)
        {
          value = std::min(value, f(mesh->vmapM[offset + v]));
        }
      }
    }
  }

  // find extreme value across all processes
  double reduced_value;
  MPI_Allreduce(&value, &reduced_value, 1, MPI_DOUBLE, MPI_MIN, platform->comm.mpiComm);

  // dimensionalize the field if needed
  solution::dimensionalize(field, reduced_value);

  // if temperature, we need to add the reference temperature
  if (field == field::temperature)
    reduced_value += scales.T_ref;

  return reduced_value;
}

Point
gllPoint(int local_elem_id, int local_node_id)
{
  mesh_t * mesh = entireMesh();

  int id = local_elem_id * mesh->Np + local_node_id;
  Point p(mesh->x[id], mesh->y[id], mesh->z[id]);
  p *= scales.L_ref;
  return p;
}

Point
gllPointFace(int local_elem_id, int local_face_id, int local_node_id)
{
  mesh_t * mesh = entireMesh();
  int face_id = mesh->EToB[local_elem_id * mesh->Nfaces + local_face_id];
  int offset = local_elem_id * mesh->Nfaces * mesh->Nfp + local_face_id * mesh->Nfp;
  int id = mesh->vmapM[offset + local_node_id];
  Point p(mesh->x[id], mesh->y[id], mesh->z[id]);
  p *= scales.L_ref;
  return p;
}

Point
centroidFace(int local_elem_id, int local_face_id)
{
  mesh_t * mesh = entireMesh();

  double x_c = 0.0;
  double y_c = 0.0;
  double z_c = 0.0;
  double mass = 0.0;

  int offset = local_elem_id * mesh->Nfaces * mesh->Nfp + local_face_id * mesh->Nfp;
  for (int v = 0; v < mesh->Np; ++v)
  {
    int id = mesh->vmapM[offset + v];
    double mass_matrix = mesh->sgeo[mesh->Nsgeo * (offset + v) + WSJID];
    x_c += mesh->x[id] * mass_matrix;
    y_c += mesh->y[id] * mass_matrix;
    z_c += mesh->z[id] * mass_matrix;
    mass += mass_matrix;
  }

  Point c(x_c, y_c, z_c);
  return c / mass * scales.L_ref;
}

Point
centroid(int local_elem_id)
{
  mesh_t * mesh = entireMesh();

  double x_c = 0.0;
  double y_c = 0.0;
  double z_c = 0.0;
  double mass = 0.0;

  for (int v = 0; v < mesh->Np; ++v)
  {
    int id = local_elem_id * mesh->Np + v;
    double mass_matrix = mesh->vgeo[local_elem_id * mesh->Np * mesh->Nvgeo + JWID * mesh->Np + v];
    x_c += mesh->x[id] * mass_matrix;
    y_c += mesh->y[id] * mass_matrix;
    z_c += mesh->z[id] * mass_matrix;
    mass += mass_matrix;
  }

  Point c(x_c, y_c, z_c);
  return c / mass * scales.L_ref;
}

double
volume()
{
  mesh_t * mesh = entireMesh();

  double integral = 0.0;

  for (int k = 0; k < mesh->Nelements; ++k)
  {
    int offset = k * mesh->Np;

    for (int v = 0; v < mesh->Np; ++v)
      integral += mesh->vgeo[mesh->Nvgeo * offset + v + mesh->Np * JWID];
  }

  // sum across all processes
  double total_integral;
  MPI_Allreduce(&integral, &total_integral, 1, MPI_DOUBLE, MPI_SUM, platform->comm.mpiComm);

  total_integral *= scales.V_ref;

  return total_integral;
}

void
dimensionalizeVolume(double & integral)
{
  integral *= scales.V_ref;
}

void
dimensionalizeArea(double & integral)
{
  integral *= scales.A_ref;
}

void
dimensionalizeVolumeIntegral(const field::NekFieldEnum & integrand,
                             const Real & volume,
                             double & integral)
{
  // dimensionalize the field if needed
  solution::dimensionalize(integrand, integral);

  // scale the volume integral
  integral *= scales.V_ref;

  // if temperature, we need to add the reference temperature multiplied by the volume integral
  if (integrand == field::temperature)
    integral += scales.T_ref * volume;
}

void
dimensionalizeSideIntegral(const field::NekFieldEnum & integrand,
                           const Real & area,
                           double & integral)
{
  // dimensionalize the field if needed
  solution::dimensionalize(integrand, integral);

  // scale the boundary integral
  integral *= scales.A_ref;

  // if temperature, we need to add the reference temperature multiplied by the area integral
  if (integrand == field::temperature)
    integral += scales.T_ref * area;
}

void
dimensionalizeSideIntegral(const field::NekFieldEnum & integrand,
                           const std::vector<int> & boundary_id,
                           double & integral)
{
  // dimensionalize the field if needed
  solution::dimensionalize(integrand, integral);

  // scale the boundary integral
  integral *= scales.A_ref;

  // if temperature, we need to add the reference temperature multiplied by the area integral
  if (integrand == field::temperature)
    integral += scales.T_ref * area(boundary_id);
}

double
volumeIntegral(const field::NekFieldEnum & integrand, const Real & volume)
{
  mesh_t * mesh = entireMesh();
  double integral = 0.0;

  double (*f)(int);
  f = solution::solutionPointer(integrand);

  for (int k = 0; k < mesh->Nelements; ++k)
  {
    int offset = k * mesh->Np;

    for (int v = 0; v < mesh->Np; ++v)
      integral += f(offset + v) * mesh->vgeo[mesh->Nvgeo * offset + v + mesh->Np * JWID];
  }

  // sum across all processes
  double total_integral;
  MPI_Allreduce(&integral, &total_integral, 1, MPI_DOUBLE, MPI_SUM, platform->comm.mpiComm);

  dimensionalizeVolumeIntegral(integrand, volume, total_integral);

  return total_integral;
}

double
area(const std::vector<int> & boundary_id)
{
  mesh_t * mesh = entireMesh();

  double integral = 0.0;

  for (int i = 0; i < mesh->Nelements; ++i)
  {
    for (int j = 0; j < mesh->Nfaces; ++j)
    {
      int face_id = mesh->EToB[i * mesh->Nfaces + j];

      if (std::find(boundary_id.begin(), boundary_id.end(), face_id) != boundary_id.end())
      {
        int offset = i * mesh->Nfaces * mesh->Nfp + j * mesh->Nfp;
        for (int v = 0; v < mesh->Nfp; ++v)
        {
          integral += mesh->sgeo[mesh->Nsgeo * (offset + v) + WSJID];
        }
      }
    }
  }

  // sum across all processes
  double total_integral;
  MPI_Allreduce(&integral, &total_integral, 1, MPI_DOUBLE, MPI_SUM, platform->comm.mpiComm);

  dimensionalizeSideIntegral(field::unity, boundary_id, total_integral);

  return total_integral;
}

double
sideIntegral(const std::vector<int> & boundary_id, const field::NekFieldEnum & integrand)
{
  mesh_t * mesh = entireMesh();

  double integral = 0.0;

  double (*f)(int);
  f = solution::solutionPointer(integrand);

  for (int i = 0; i < mesh->Nelements; ++i)
  {
    for (int j = 0; j < mesh->Nfaces; ++j)
    {
      int face_id = mesh->EToB[i * mesh->Nfaces + j];

      if (std::find(boundary_id.begin(), boundary_id.end(), face_id) != boundary_id.end())
      {
        int offset = i * mesh->Nfaces * mesh->Nfp + j * mesh->Nfp;
        for (int v = 0; v < mesh->Nfp; ++v)
        {
          integral += f(mesh->vmapM[offset + v]) * mesh->sgeo[mesh->Nsgeo * (offset + v) + WSJID];
        }
      }
    }
  }

  // sum across all processes
  double total_integral;
  MPI_Allreduce(&integral, &total_integral, 1, MPI_DOUBLE, MPI_SUM, platform->comm.mpiComm);

  dimensionalizeSideIntegral(integrand, boundary_id, total_integral);

  return total_integral;
}

double
massFlowrate(const std::vector<int> & boundary_id)
{
  nrs_t * nrs = (nrs_t *)nrsPtr();
  mesh_t * mesh = entireMesh();

  // TODO: This function only works correctly if the density is constant, because
  // otherwise we need to copy the density from device to host
  double rho;
  platform->options.getArgs("DENSITY", rho);

  double integral = 0.0;

  for (int i = 0; i < mesh->Nelements; ++i)
  {
    for (int j = 0; j < mesh->Nfaces; ++j)
    {
      int face_id = mesh->EToB[i * mesh->Nfaces + j];

      if (std::find(boundary_id.begin(), boundary_id.end(), face_id) != boundary_id.end())
      {
        int offset = i * mesh->Nfaces * mesh->Nfp + j * mesh->Nfp;
        for (int v = 0; v < mesh->Nfp; ++v)
        {
          int vol_id = mesh->vmapM[offset + v];
          int surf_offset = mesh->Nsgeo * (offset + v);

          double normal_velocity =
              nrs->U[vol_id + 0 * velocityFieldOffset()] * mesh->sgeo[surf_offset + NXID] +
              nrs->U[vol_id + 1 * velocityFieldOffset()] * mesh->sgeo[surf_offset + NYID] +
              nrs->U[vol_id + 2 * velocityFieldOffset()] * mesh->sgeo[surf_offset + NZID];

          integral += rho * normal_velocity * mesh->sgeo[surf_offset + WSJID];
        }
      }
    }
  }

  // sum across all processes
  double total_integral;
  MPI_Allreduce(&integral, &total_integral, 1, MPI_DOUBLE, MPI_SUM, platform->comm.mpiComm);

  // dimensionalize the mass flux and area
  total_integral *= scales.rho_ref * scales.U_ref * scales.A_ref;

  return total_integral;
}

double
sideMassFluxWeightedIntegral(const std::vector<int> & boundary_id,
                             const field::NekFieldEnum & integrand)
{
  nrs_t * nrs = (nrs_t *)nrsPtr();
  mesh_t * mesh = entireMesh();

  // TODO: This function only works correctly if the density is constant, because
  // otherwise we need to copy the density from device to host
  double rho;
  platform->options.getArgs("DENSITY", rho);

  double integral = 0.0;

  double (*f)(int);
  f = solution::solutionPointer(integrand);

  for (int i = 0; i < mesh->Nelements; ++i)
  {
    for (int j = 0; j < mesh->Nfaces; ++j)
    {
      int face_id = mesh->EToB[i * mesh->Nfaces + j];

      if (std::find(boundary_id.begin(), boundary_id.end(), face_id) != boundary_id.end())
      {
        int offset = i * mesh->Nfaces * mesh->Nfp + j * mesh->Nfp;
        for (int v = 0; v < mesh->Nfp; ++v)
        {
          int vol_id = mesh->vmapM[offset + v];
          int surf_offset = mesh->Nsgeo * (offset + v);
          double normal_velocity =
              nrs->U[vol_id + 0 * velocityFieldOffset()] * mesh->sgeo[surf_offset + NXID] +
              nrs->U[vol_id + 1 * velocityFieldOffset()] * mesh->sgeo[surf_offset + NYID] +
              nrs->U[vol_id + 2 * velocityFieldOffset()] * mesh->sgeo[surf_offset + NZID];
          integral += f(vol_id) * rho * normal_velocity * mesh->sgeo[surf_offset + WSJID];
        }
      }
    }
  }

  // sum across all processes
  double total_integral;
  MPI_Allreduce(&integral, &total_integral, 1, MPI_DOUBLE, MPI_SUM, platform->comm.mpiComm);

  // dimensionalize the field if needed
  solution::dimensionalize(integrand, total_integral);

  // dimensionalize the mass flux and area
  total_integral *= scales.rho_ref * scales.U_ref * scales.A_ref;

  // if temperature, we need to add the reference temperature multiplied by the mass flux integral
  if (integrand == field::temperature)
    total_integral += scales.T_ref * massFlowrate(boundary_id);

  return total_integral;
}

double
heatFluxIntegral(const std::vector<int> & boundary_id)
{
  nrs_t * nrs = (nrs_t *)nrsPtr();
  mesh_t * mesh = temperatureMesh();

  // TODO: This function only works correctly if the conductivity is constant, because
  // otherwise we need to copy the conductivity from device to host
  double k;
  platform->options.getArgs("SCALAR00 DIFFUSIVITY", k);

  double integral = 0.0;

  double * grad_T = (double *)calloc(3 * scalarFieldOffset(), sizeof(double));
  gradient(scalarFieldOffset(), nrs->cds->S, grad_T);

  for (int i = 0; i < mesh->Nelements; ++i)
  {
    for (int j = 0; j < mesh->Nfaces; ++j)
    {
      int face_id = mesh->EToB[i * mesh->Nfaces + j];

      if (std::find(boundary_id.begin(), boundary_id.end(), face_id) != boundary_id.end())
      {
        int offset = i * mesh->Nfaces * mesh->Nfp + j * mesh->Nfp;
        for (int v = 0; v < mesh->Nfp; ++v)
        {
          int vol_id = mesh->vmapM[offset + v];
          int surf_offset = mesh->Nsgeo * (offset + v);

          double normal_grad_T =
              grad_T[vol_id + 0 * scalarFieldOffset()] * mesh->sgeo[surf_offset + NXID] +
              grad_T[vol_id + 1 * scalarFieldOffset()] * mesh->sgeo[surf_offset + NYID] +
              grad_T[vol_id + 2 * scalarFieldOffset()] * mesh->sgeo[surf_offset + NZID];

          integral += -k * normal_grad_T * mesh->sgeo[surf_offset + WSJID];
        }
      }
    }
  }

  freePointer(grad_T);

  // sum across all processes
  double total_integral;
  MPI_Allreduce(&integral, &total_integral, 1, MPI_DOUBLE, MPI_SUM, platform->comm.mpiComm);

  // multiply by the reference heat flux and an area factor to dimensionalize
  total_integral *= scales.flux_ref * scales.A_ref;

  return total_integral;
}

void
gradient(const int offset, const double * f, double * grad_f)
{
  mesh_t * mesh = entireMesh();

  std::vector<std::vector<std::vector<double>>> s_P(
      mesh->Nq, std::vector<std::vector<double>>(mesh->Nq, std::vector<double>(mesh->Nq, 0)));
  std::vector<std::vector<double>> s_D(mesh->Nq, std::vector<double>(mesh->Nq, 0));

  for (int e = 0; e < mesh->Nelements; ++e)
  {
    for (int k = 0; k < mesh->Nq; ++k)
      for (int j = 0; j < mesh->Nq; ++j)
        for (int i = 0; i < mesh->Nq; ++i)
        {
          const int id = e * mesh->Np + k * mesh->Nq * mesh->Nq + j * mesh->Nq + i;

          s_P[k][j][i] = f[id];

          if (k == 0)
            s_D[j][i] = mesh->D[j * mesh->Nq + i];
        }

    for (int k = 0; k < mesh->Nq; ++k)
    {
      for (int j = 0; j < mesh->Nq; ++j)
      {
        for (int i = 0; i < mesh->Nq; ++i)
        {
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
          double dpdr = 0.f, dpds = 0.f, dpdt = 0.f;

          for (int n = 0; n < mesh->Nq; ++n)
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

bool
isHeatFluxBoundary(const int boundary)
{
  // the heat flux boundary is now named 'codedFixedGradient', but 'fixedGradient'
  // will be present for backwards compatibility
  return (bcMap::text(boundary, "scalar00") == "fixedGradient") ||
         (bcMap::text(boundary, "scalar00") == "codedFixedGradient");
}

bool
isTemperatureBoundary(const int boundary)
{
  return bcMap::text(boundary, "scalar00") == "fixedValue";
}

const std::string
temperatureBoundaryType(const int boundary)
{
  return bcMap::text(boundary, "scalar00");
}

int
polynomialOrder()
{
  return entireMesh()->N;
}

int
Nelements()
{
  int n_local = entireMesh()->Nelements;
  int n_global;
  MPI_Allreduce(&n_local, &n_global, 1, MPI_INT, MPI_SUM, platform->comm.mpiComm);
  return n_global;
}

int
Nfaces()
{
  return entireMesh()->Nfaces;
}

int
dim()
{
  return entireMesh()->dim;
}

int
NfaceVertices()
{
  return entireMesh()->NfaceVertices;
}

int
NboundaryFaces()
{
  return entireMesh()->NboundaryFaces;
}

int
NboundaryID()
{
  if (entireMesh()->cht)
    return nekData.NboundaryIDt;
  else
    return nekData.NboundaryID;
}

bool
validBoundaryIDs(const std::vector<int> & boundary_id, int & first_invalid_id, int & n_boundaries)
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

} // end namespace mesh

namespace solution
{

double
scalar01(const int id)
{
  nrs_t * nrs = (nrs_t *)nrsPtr();
  return nrs->cds->S[id + 1 * scalarFieldOffset()];
}

double
scalar02(const int id)
{
  nrs_t * nrs = (nrs_t *)nrsPtr();
  return nrs->cds->S[id + 2 * scalarFieldOffset()];
}

double
scalar03(const int id)
{
  nrs_t * nrs = (nrs_t *)nrsPtr();
  return nrs->cds->S[id + 3 * scalarFieldOffset()];
}

double
temperature(const int id)
{
  nrs_t * nrs = (nrs_t *)nrsPtr();
  return nrs->cds->S[id];
}

double
pressure(const int id)
{
  nrs_t * nrs = (nrs_t *)nrsPtr();
  return nrs->P[id];
}

double
unity(const int /* id */)
{
  return 1.0;
}

double
velocity_x(const int id)
{
  nrs_t * nrs = (nrs_t *)nrsPtr();
  return nrs->U[id + 0 * nrs->fieldOffset];
}

double
velocity_y(const int id)
{
  nrs_t * nrs = (nrs_t *)nrsPtr();
  return nrs->U[id + 1 * nrs->fieldOffset];
}

double
velocity_z(const int id)
{
  nrs_t * nrs = (nrs_t *)nrsPtr();
  return nrs->U[id + 2 * nrs->fieldOffset];
}

double
velocity(const int id)
{
  nrs_t * nrs = (nrs_t *)nrsPtr();
  int offset = nrs->fieldOffset;

  return std::sqrt(nrs->U[id + 0 * offset] * nrs->U[id + 0 * offset] +
                   nrs->U[id + 1 * offset] * nrs->U[id + 1 * offset] +
                   nrs->U[id + 2 * offset] * nrs->U[id + 2 * offset]);
}

void
flux(const int id, const dfloat value)
{
  nrs_t * nrs = (nrs_t *)nrsPtr();
  nrs->usrwrk[id] = value;
}

void
heat_source(const int id, const dfloat value)
{
  nrs_t * nrs = (nrs_t *)nrsPtr();
  nrs->usrwrk[1 * scalarFieldOffset() + id] = value;
}

void
x_displacement(const int id, const dfloat value)
{
  mesh_t * mesh = entireMesh();
  mesh->x[id] = value;
}

void
y_displacement(const int id, const dfloat value)
{
  mesh_t * mesh = entireMesh();
  mesh->y[id] = value;
}

void
z_displacement(const int id, const dfloat value)
{
  mesh_t * mesh = entireMesh();
  mesh->z[id] = value;
}

double (*solutionPointer(const field::NekFieldEnum & field))(int)
{
  double (*f)(int);

  switch (field)
  {
    case field::velocity_x:
      f = &solution::velocity_x;
      break;
    case field::velocity_y:
      f = &solution::velocity_y;
      break;
    case field::velocity_z:
      f = &solution::velocity_z;
      break;
    case field::velocity:
      f = &solution::velocity;
      break;
    case field::velocity_component:
      mooseError("The 'velocity_component' field is not compatible with the solutionPointer "
                 "interface!");
      break;
    case field::temperature:
      if (!nekrs::hasTemperatureVariable())
        mooseError("Cardinal cannot find 'temperature' "
                   "because your Nek case files do not have a temperature variable!");
      f = &solution::temperature;
      break;
    case field::pressure:
      f = &solution::pressure;
      break;
    case field::scalar01:
      if (!hasScalarVariable(1))
        mooseError("Cardinal cannot find 'scalar01' "
                   "because your Nek case files do not have a scalar01 variable!");
      f = &solution::scalar01;
      break;
    case field::scalar02:
      if (!hasScalarVariable(2))
        mooseError("Cardinal cannot find 'scalar02' "
                   "because your Nek case files do not have a scalar02 variable!");
      f = &solution::scalar02;
      break;
    case field::scalar03:
      if (!hasScalarVariable(3))
        mooseError("Cardinal cannot find 'scalar03' "
                   "because your Nek case files do not have a scalar03 variable!");
      f = &solution::scalar03;
      break;
    case field::unity:
      f = &solution::unity;
      break;
    default:
      throw std::runtime_error("Unhandled 'NekFieldEnum'!");
  }

  return f;
}

void (*solutionPointer(const field::NekWriteEnum & field))(int, dfloat)
{
  void (*f)(int, dfloat);

  switch (field)
  {
    case field::flux:
      f = &solution::flux;
      break;
    case field::heat_source:
      f = &solution::heat_source;
      break;
    case field::x_displacement:
      f = &solution::x_displacement;
      break;
    case field::y_displacement:
      f = &solution::y_displacement;
      break;
    case field::z_displacement:
      f = &solution::z_displacement;
      break;
    default:
      throw std::runtime_error("Unhandled NekWriteEnum!");
  }

  return f;
}

void
initializeDimensionalScales(const double U_ref,
                            const double T_ref,
                            const double dT_ref,
                            const double L_ref,
                            const double rho_ref,
                            const double Cp_ref)
{
  scales.U_ref = U_ref;
  scales.T_ref = T_ref;
  scales.dT_ref = dT_ref;
  scales.L_ref = L_ref;
  scales.A_ref = L_ref * L_ref;
  scales.V_ref = L_ref * L_ref * L_ref;
  scales.rho_ref = rho_ref;
  scales.Cp_ref = Cp_ref;

  scales.flux_ref = rho_ref * U_ref * Cp_ref * dT_ref;
  scales.source_ref = scales.flux_ref / L_ref;

  scales.nondimensional_T = (std::abs(dT_ref - 1.0) > abs_tol) || (std::abs(T_ref) > abs_tol);
}

double
referenceFlux()
{
  return scales.flux_ref;
}

double
referenceSource()
{
  return scales.source_ref;
}

double
referenceLength()
{
  return scales.L_ref;
}

double
referenceArea()
{
  return scales.A_ref;
}

void
dimensionalize(const field::NekFieldEnum & field, double & value)
{
  switch (field)
  {
    case field::velocity_x:
      value = value * scales.U_ref;
      break;
    case field::velocity_y:
      value = value * scales.U_ref;
      break;
    case field::velocity_z:
      value = value * scales.U_ref;
      break;
    case field::velocity:
      value = value * scales.U_ref;
      break;
    case field::velocity_component:
      mooseError(
          "The 'velocity_component' field is incompatible with the dimensionalize interface!");
      break;
    case field::temperature:
      value = value * scales.dT_ref;
      break;
    case field::pressure:
      value = value * scales.rho_ref * scales.U_ref * scales.U_ref;
      break;
    case field::scalar01:
      // no dimensionalization needed
      break;
    case field::scalar02:
      // no dimensionalization needed
      break;
    case field::scalar03:
      // no dimensionalization needed
      break;
    case field::unity:
      // no dimensionalization needed
      break;
    default:
      throw std::runtime_error("Unhandled 'NekFieldEnum'!");
  }
}
} // end namespace solution

template <>
MPI_Datatype
resolveType<double>()
{
  return MPI_DOUBLE;
}

template <>
MPI_Datatype
resolveType<int>()
{
  return MPI_INT;
}

} // end namespace nekrs

#endif
