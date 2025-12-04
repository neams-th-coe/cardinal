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

static nekrs::characteristicScales scales;
static dfloat * sgeo;
static dfloat * vgeo;
static unsigned int n_usrwrk_slots;
static bool is_nondimensional;

namespace nekrs
{
static double setup_time;

// various constants for controlling tolerances
static double abs_tol;
static double rel_tol;

void
setAbsoluteTol(double tol)
{
  abs_tol = tol;
}

void
setRelativeTol(double tol)
{
  rel_tol = tol;
}

void
setNekSetupTime(const double & time)
{
  setup_time = time;
}

double
getNekSetupTime()
{
  return setup_time;
}

void
setStartTime(const double & start)
{
  platform->options.setArgs("START TIME", to_string_f(start));
}

void
write_usrwrk_field_file(const int & slot, const std::string & prefix, const dfloat & time, const int & step, const bool & write_coords)
{
  int num_bytes = fieldOffset() * sizeof(dfloat);

  nrs_t * nrs = (nrs_t *)nrsPtr();
  occa::memory o_write = platform->device.malloc(num_bytes);
  o_write.copyFrom(nrs->bc->o_usrwrk, num_bytes /* length we are copying */,
    0 /* where to place data */, num_bytes * slot /* where to source data */);

  occa::memory o_null;
  //TODO
  //nek::writeFld(prefix.c_str(), time, step, write_coords, 1 /* FP64 */, o_null, o_null, o_write, 1);
}

void
write_field_file(const std::string & prefix, const dfloat time, const int & step)
{
  nrs_t * nrs = (nrs_t *)nrsPtr();

  int Nscalar = 0;
  occa::memory o_s;
  if (nrs->Nscalar)
  {
    o_s = nrs->scalar->o_S;
    Nscalar = nrs->Nscalar;
  }

  //TODO
  //nek::writeFld(prefix.c_str(), time, step, 1 /* coords */, 1 /* FP64 */, nrs->fluid->o_U, nrs->fluid->o_P, o_s, Nscalar);
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
  nrs_t * nrs = (nrs_t *)nrsPtr();

  for (int is = 0; is < nrs->Nscalar; is++) {
    if (platform->options.compareArgs("SCALAR" + scalarDigitStr(is) + " MESH", "SOLID")) {
      return true;
    }
  }
  return false;
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
hasBlendingSolver()
{
  return !platform->options.compareArgs("MESH SOLVER", "NONE") && hasMovingMesh();
}

bool
hasUserMeshSolver()
{
  return platform->options.compareArgs("MESH SOLVER", "NONE") && hasMovingMesh();
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
  return hasTemperatureVariable() ? nrs->scalar->compute[0] : false;
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
  nrs_t * nrs = (nrs_t *)nrsPtr();
  return static_cast<bool>(nrs->userSource);
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
  return nrs->scalar->fieldOffset(); //same for all scalars
}

int
velocityFieldOffset()
{
  nrs_t * nrs = (nrs_t *)nrsPtr();
  return nrs->fieldOffset;
}

int
fieldOffset()
{
  if (hasTemperatureVariable())
    return scalarFieldOffset();
  else
    return velocityFieldOffset();
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
  return nrs->scalar->mesh("temperature");
}

mesh_t *
getMesh(const nek_mesh::NekMeshEnum pp_mesh)
{
  if (pp_mesh == nek_mesh::fluid)
    return flowMesh();
  else if (pp_mesh == nek_mesh::all)
    return entireMesh();
  else
    mooseError("This object does not support operations on the solid part of the NekRS mesh!\n"
      "Valid options for 'mesh' are 'fluid' or 'all'.");
}

int
commRank()
{
  return platform->comm.mpiRank();
}

int
commSize()
{
  return platform->comm.mpiCommSize();
}

bool
scratchAvailable()
{
  nrs_t * nrs = (nrs_t *)nrsPtr();

  // Because these scratch spaces are available for whatever the user sees fit, it is
  // possible that the user wants to use these arrays for a _different_ purpose aside from
  // transferring in MOOSE values. In nekrs::setup, we call the UDF_Setup0, UDF_Setup,
  // and UDF_ExecuteStep routines. These scratch space arrays aren't initialized anywhere
  // else in the core base, so we will make sure to throw an error from MOOSE if these
  // arrays are already in use, because otherwise our MOOSE transfer might get overwritten
  // by whatever other operation the user is trying to do.
  if (nrs->bc->usrwrk)
    return false;

  return true;
}

void
initializeScratch(const unsigned int & n_slots)
{
  if (n_slots == 0)
    return;

  nrs_t * nrs = (nrs_t *)nrsPtr();
  mesh_t * mesh = entireMesh();

  // clear them just to be sure
  freeScratch();

  // In order to make indexing simpler in the device user functions (which is where the
  // boundary conditions are then actually applied), we define these scratch arrays
  // as volume arrays.
  usrwrk = (double *)calloc(n_slots * fieldOffset(), sizeof(double));
  nrs->bc->o_usrwrk = platform->device.malloc(n_slots * fieldOffset() * sizeof(double), usrwrk);

  n_usrwrk_slots = n_slots;
}

void
freeScratch()
{
  nrs_t * nrs = (nrs_t *)nrsPtr();

  freePointer(usrwrk);
  nrs->bc->o_usrwrk.free();
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
usrwrkVolumeIntegral(const unsigned int & slot, const nek_mesh::NekMeshEnum pp_mesh)
{
  nrs_t * nrs = (nrs_t *)nrsPtr();
  const auto & mesh = getMesh(pp_mesh);

  double integral = 0.0;

  for (int k = 0; k < mesh->Nelements; ++k)
  {
    int offset = k * mesh->Np;

    for (int v = 0; v < mesh->Np; ++v)
      integral += usrwrk[slot + offset + v] * vgeo[mesh->Nvgeo * offset + v + mesh->Np * JWID];
  }

  // sum across all processes
  double total_integral;
  MPI_Allreduce(&integral, &total_integral, 1, MPI_DOUBLE, MPI_SUM, platform->comm.mpiComm());

  return total_integral;
}

void
scaleUsrwrk(const unsigned int & slot, const dfloat & value)
{
  nrs_t * nrs = (nrs_t *)nrsPtr();
  mesh_t * mesh = getMesh(nek_mesh::all);

  for (int k = 0; k < mesh->Nelements; ++k)
  {
    int id = k * mesh->Np;

    for (int v = 0; v < mesh->Np; ++v)
      usrwrk[slot + id + v] *= value;
  }
}

std::vector<double>
usrwrkSideIntegral(const unsigned int & slot,
                   const std::vector<int> & boundary,
                   const nek_mesh::NekMeshEnum pp_mesh)
{
  nrs_t * nrs = (nrs_t *)nrsPtr();
  const auto & mesh = getMesh(pp_mesh);

  std::vector<double> integral(boundary.size(), 0.0);

  for (int i = 0; i < mesh->Nelements; ++i)
  {
    for (int j = 0; j < mesh->Nfaces; ++j)
    {
      int face_id = mesh->EToB[i * mesh->Nfaces + j];

      if (std::find(boundary.begin(), boundary.end(), face_id) != boundary.end())
      {
        auto it = std::find(boundary.begin(), boundary.end(), face_id);
        auto b_index = it - boundary.begin();

        int offset = i * mesh->Nfaces * mesh->Nfp + j * mesh->Nfp;

        for (int v = 0; v < mesh->Nfp; ++v)
          integral[b_index] += usrwrk[slot + mesh->vmapM[offset + v]] *
                               sgeo[mesh->Nsgeo * (offset + v) + WSJID];
      }
    }
  }

  // sum across all processes; this can probably be done more efficiently
  std::vector<double> total_integral(boundary.size(), 0.0);
  for (std::size_t i = 0; i < boundary.size(); ++i)
    MPI_Allreduce(&integral[i], &total_integral[i], 1, MPI_DOUBLE, MPI_SUM, platform->comm.mpiComm());

  return total_integral;
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

  const auto sid = nrs->scalar->nameToIndex.find("temperature")->second;
  const auto offset = nrs->scalar->fieldOffset();

  for (int i = 0; i < mesh->Nelements; ++i)
  {
    for (int j = 0; j < mesh->Np; ++j)
    {
      int id = i * mesh->Np + j + sid * offset;

      if (S[id] < minimum)
        S[id] = minimum;
      if (S[id] > maximum)
        S[id] = maximum;
    }
  }

  // when complete, copy to device
  auto o_temperature = nrs->scalar->o_solution("temperature");
  o_temperature.copyFrom(S, offset, sid * offset, 0);
}

void
copyDeformationToDevice()
{
  mesh_t * mesh = entireMesh();

  mesh->o_x.copyFrom(x.data());
  mesh->o_y.copyFrom(y.data());
  mesh->o_z.copyFrom(z.data());
  mesh->update();

  updateHostMeshParameters();
}

void
initializeHostMeshParameters()
{
  mesh_t * mesh = entireMesh();
  sgeo = (dfloat *)calloc(mesh->o_sgeo.size(), sizeof(dfloat));
  vgeo = (dfloat *)calloc(mesh->o_vgeo.size(), sizeof(dfloat));
}

void
updateHostMeshParameters()
{
  mesh_t * mesh = entireMesh();
  mesh->o_sgeo.copyTo(sgeo);
  mesh->o_vgeo.copyTo(vgeo);
}

dfloat *
getSgeo()
{
  return sgeo;
}

dfloat *
getVgeo()
{
  return vgeo;
}

double
sideExtremeValue(const std::vector<int> & boundary_id, const field::NekFieldEnum & field,
             const nek_mesh::NekMeshEnum pp_mesh, const bool max)
{
  mesh_t * mesh = getMesh(pp_mesh);

  double value = max ? -std::numeric_limits<double>::max() : std::numeric_limits<double>::max();

  double (*f)(int, int);
  f = solutionPointer(field);

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
          if (max)
            value = std::max(value, f(mesh->vmapM[offset + v], 0 /* unused */));
          else
            value = std::min(value, f(mesh->vmapM[offset + v], 0 /* unused */));
        }
      }
    }
  }

  // find extreme value across all processes
  double reduced_value;
  auto op = max ? MPI_MAX : MPI_MIN;
  MPI_Allreduce(&value, &reduced_value, 1, MPI_DOUBLE, op, platform->comm.mpiComm());

  // dimensionalize the field if needed
  reduced_value = reduced_value * nondimensionalDivisor(field) + nondimensionalAdditive(field);

  return reduced_value;
}

double
volumeExtremeValue(const field::NekFieldEnum & field, const nek_mesh::NekMeshEnum pp_mesh, const bool max)
{
  double value = max ? -std::numeric_limits<double>::max() : std::numeric_limits<double>::max();

  double (*f)(int, int);
  f = solutionPointer(field);

  mesh_t * mesh;
  int start_id;

  switch (pp_mesh)
  {
    case nek_mesh::fluid:
    case nek_mesh::all:
    {
      mesh = getMesh(pp_mesh);
      start_id = 0;
      break;
    }
    case nek_mesh::solid:
    {
      mesh = entireMesh();
      start_id = flowMesh()->Nelements;
      break;
    }
    default:
      mooseError("Unhandled NekMeshEnum in volumeExtremeValue");
  }

  for (int i = start_id; i < mesh->Nelements; ++i)
  {
    for (int j = 0; j < mesh->Np; ++j)
    {
      if (max)
        value = std::max(value, f(i * mesh->Np + j, 0 /* unused */));
      else
        value = std::min(value, f(i * mesh->Np + j, 0 /* unused */));
    }
  }

  // find extreme value across all processes
  double reduced_value;
  auto op = max ? MPI_MAX : MPI_MIN;
  MPI_Allreduce(&value, &reduced_value, 1, MPI_DOUBLE, op, platform->comm.mpiComm());

  // dimensionalize the field if needed
  reduced_value = reduced_value * nondimensionalDivisor(field) + nondimensionalAdditive(field);

  return reduced_value;
}

Point
gllPoint(int local_elem_id, int local_node_id)
{
  mesh_t * mesh = entireMesh();

  int id = local_elem_id * mesh->Np + local_node_id;
  Point p(x[id], y[id], z[id]);
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

  Point p(x[id], y[id], z[id]);
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
    double mass_matrix = sgeo[mesh->Nsgeo * (offset + v) + WSJID];
    x_c += x[id] * mass_matrix;
    y_c += y[id] * mass_matrix;
    z_c += z[id] * mass_matrix;
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
    double mass_matrix = vgeo[local_elem_id * mesh->Np * mesh->Nvgeo + JWID * mesh->Np + v];
    x_c += x[id] * mass_matrix;
    y_c += y[id] * mass_matrix;
    z_c += z[id] * mass_matrix;
    mass += mass_matrix;
  }

  Point c(x_c, y_c, z_c);
  return c / mass * scales.L_ref;
}

double
volume(const nek_mesh::NekMeshEnum pp_mesh)
{
  mesh_t * mesh = getMesh(pp_mesh);
  double integral = 0.0;

  for (int k = 0; k < mesh->Nelements; ++k)
  {
    int offset = k * mesh->Np;

    for (int v = 0; v < mesh->Np; ++v)
      integral += vgeo[mesh->Nvgeo * offset + v + mesh->Np * JWID];
  }

  // sum across all processes
  double total_integral;
  MPI_Allreduce(&integral, &total_integral, 1, MPI_DOUBLE, MPI_SUM, platform->comm.mpiComm());

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
  integral *= nondimensionalDivisor(integrand);

  // scale the volume integral
  integral *= scales.V_ref;

  // for quantities with a relative scaling, we need to add back the reference
  // contribution to the volume integral
  integral += nondimensionalAdditive(integrand) * volume;
}

void
dimensionalizeSideIntegral(const field::NekFieldEnum & integrand,
                           const Real & area,
                           double & integral)
{
  // dimensionalize the field if needed
  integral *= nondimensionalDivisor(integrand);

  // scale the boundary integral
  integral *= scales.A_ref;

  // for quantities with a relative scaling, we need to add back the reference
  // contribution to the side integral
  integral += nondimensionalAdditive(integrand) * area;
}

void
dimensionalizeSideIntegral(const field::NekFieldEnum & integrand,
                           const std::vector<int> & boundary_id,
                           double & integral,
			                     const nek_mesh::NekMeshEnum pp_mesh)
{
  // dimensionalize the field if needed
  integral *= nondimensionalDivisor(integrand);

  // scale the boundary integral
  integral *= scales.A_ref;

  // for quantities with a relative scaling, we need to add back the reference
  // contribution to the side integral; we need this form here to avoid a recursive loop
  auto add = nondimensionalAdditive(integrand);
  if (std::abs(add) > 1e-8)
    integral += add * area(boundary_id, pp_mesh);
}

double
volumeIntegral(const field::NekFieldEnum & integrand, const Real & volume,
               const nek_mesh::NekMeshEnum pp_mesh)
{
  mesh_t * mesh = getMesh(pp_mesh);

  double integral = 0.0;

  double (*f)(int, int);
  f = solutionPointer(integrand);

  for (int k = 0; k < mesh->Nelements; ++k)
  {
    int offset = k * mesh->Np;

    for (int v = 0; v < mesh->Np; ++v)
      integral += f(offset + v, 0 /* unused */) * vgeo[mesh->Nvgeo * offset + v + mesh->Np * JWID];
  }

  // sum across all processes
  double total_integral;
  MPI_Allreduce(&integral, &total_integral, 1, MPI_DOUBLE, MPI_SUM, platform->comm.mpiComm());

  dimensionalizeVolumeIntegral(integrand, volume, total_integral);

  return total_integral;
}

double
area(const std::vector<int> & boundary_id, const nek_mesh::NekMeshEnum pp_mesh)
{
  mesh_t * mesh = getMesh(pp_mesh);

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
          integral += sgeo[mesh->Nsgeo * (offset + v) + WSJID];
        }
      }
    }
  }

  // sum across all processes
  double total_integral;
  MPI_Allreduce(&integral, &total_integral, 1, MPI_DOUBLE, MPI_SUM, platform->comm.mpiComm());

  dimensionalizeSideIntegral(field::unity, boundary_id, total_integral, pp_mesh);

  return total_integral;
}

double
sideIntegral(const std::vector<int> & boundary_id, const field::NekFieldEnum & integrand,
             const nek_mesh::NekMeshEnum pp_mesh)
{
  mesh_t * mesh = getMesh(pp_mesh);

  double integral = 0.0;

  double (*f)(int, int);
  f = solutionPointer(integrand);

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
          integral +=
              f(mesh->vmapM[offset + v], 0 /* unused */) * sgeo[mesh->Nsgeo * (offset + v) + WSJID];
        }
      }
    }
  }

  // sum across all processes
  double total_integral;
  MPI_Allreduce(&integral, &total_integral, 1, MPI_DOUBLE, MPI_SUM, platform->comm.mpiComm());

  dimensionalizeSideIntegral(integrand, boundary_id, total_integral, pp_mesh);

  return total_integral;
}

double
massFlowrate(const std::vector<int> & boundary_id, const nek_mesh::NekMeshEnum pp_mesh)
{
  mesh_t * mesh = getMesh(pp_mesh);
  nrs_t * nrs = (nrs_t *)nrsPtr();

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
              U[vol_id + 0 * velocityFieldOffset()] * sgeo[surf_offset + NXID] +
              U[vol_id + 1 * velocityFieldOffset()] * sgeo[surf_offset + NYID] +
              U[vol_id + 2 * velocityFieldOffset()] * sgeo[surf_offset + NZID];

          integral += rho * normal_velocity * sgeo[surf_offset + WSJID];
        }
      }
    }
  }

  // sum across all processes
  double total_integral;
  MPI_Allreduce(&integral, &total_integral, 1, MPI_DOUBLE, MPI_SUM, platform->comm.mpiComm());

  // dimensionalize the mass flux and area
  total_integral *= scales.rho_ref * scales.U_ref * scales.A_ref;

  return total_integral;
}

double
sideMassFluxWeightedIntegral(const std::vector<int> & boundary_id,
                             const field::NekFieldEnum & integrand,
                             const nek_mesh::NekMeshEnum pp_mesh)
{
  mesh_t * mesh = getMesh(pp_mesh);
  nrs_t * nrs = (nrs_t *)nrsPtr();

  // TODO: This function only works correctly if the density is constant, because
  // otherwise we need to copy the density from device to host
  double rho;
  platform->options.getArgs("DENSITY", rho);

  double integral = 0.0;

  double (*f)(int, int);
  f = solutionPointer(integrand);

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
              U[vol_id + 0 * velocityFieldOffset()] * sgeo[surf_offset + NXID] +
              U[vol_id + 1 * velocityFieldOffset()] * sgeo[surf_offset + NYID] +
              U[vol_id + 2 * velocityFieldOffset()] * sgeo[surf_offset + NZID];
          integral += f(vol_id, 0 /* unused */) * rho * normal_velocity * sgeo[surf_offset + WSJID];
        }
      }
    }
  }

  // sum across all processes
  double total_integral;
  MPI_Allreduce(&integral, &total_integral, 1, MPI_DOUBLE, MPI_SUM, platform->comm.mpiComm());

  // dimensionalize the field if needed
  total_integral *= nondimensionalDivisor(integrand);

  // dimensionalize the mass flux and area
  total_integral *= scales.rho_ref * scales.U_ref * scales.A_ref;

  // for quantities with a relative scaling, we need to add back the reference
  // contribution to the mass flux integral; we need this form here to avoid an infinite
  // recursive loop
  auto add = nondimensionalAdditive(integrand);
  if (std::abs(add) > 1e-8)
    total_integral += add * massFlowrate(boundary_id, pp_mesh);

  return total_integral;
}

double
pressureSurfaceForce(const std::vector<int> & boundary_id, const Point & direction, const nek_mesh::NekMeshEnum pp_mesh)
{
  mesh_t * mesh = getMesh(pp_mesh);
  nrs_t * nrs = (nrs_t *)nrsPtr();

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

          double p_normal = P[vol_id] * (sgeo[surf_offset + NXID] * direction(0) +
                                         sgeo[surf_offset + NYID] * direction(1) +
                                         sgeo[surf_offset + NZID] * direction(2));

          integral += p_normal * sgeo[surf_offset + WSJID];
        }
      }
    }
  }

  // sum across all processes
  double total_integral;
  MPI_Allreduce(&integral, &total_integral, 1, MPI_DOUBLE, MPI_SUM, platform->comm.mpiComm());

  dimensionalizeSideIntegral(field::pressure, boundary_id, total_integral, pp_mesh);

  return total_integral;
}

double
heatFluxIntegral(const std::vector<int> & boundary_id, const nek_mesh::NekMeshEnum pp_mesh)
{
  mesh_t * mesh = getMesh(pp_mesh);
  nrs_t * nrs = (nrs_t *)nrsPtr();

  // TODO: This function only works correctly if the conductivity is constant, because
  // otherwise we need to copy the conductivity from device to host
  double k;
  platform->options.getArgs("SCALAR00 DIFFUSIVITY", k);

  double integral = 0.0;
  double * grad_T = (double *)calloc(3 * mesh->Np, sizeof(double));

  const auto sid = nrs->scalar->nameToIndex.find("temperature")->second;
  const auto offset = nrs->scalar->fieldOffset();
  std::vector<dfloat> temperature(S.begin() + sid * offset, S.begin() + (sid + 1) * offset);

  for (int i = 0; i < mesh->Nelements; ++i)
  {
    for (int j = 0; j < mesh->Nfaces; ++j)
    {
      int face_id = mesh->EToB[i * mesh->Nfaces + j];

      if (std::find(boundary_id.begin(), boundary_id.end(), face_id) != boundary_id.end())
      {
        // some inefficiency if an element has more than one face on the sideset of interest,
        // because we will recompute the gradient in the element more than one time - but this
        // is of little practical interest because this will be a minority of cases.
        gradient(mesh->Np, i, temperature.data(), grad_T, pp_mesh);

        int offset = i * mesh->Nfaces * mesh->Nfp + j * mesh->Nfp;
        for (int v = 0; v < mesh->Nfp; ++v)
        {
          int vol_id = mesh->vmapM[offset + v] - i * mesh->Np;
          int surf_offset = mesh->Nsgeo * (offset + v);

          double normal_grad_T = grad_T[vol_id + 0 * mesh->Np] * sgeo[surf_offset + NXID] +
                                 grad_T[vol_id + 1 * mesh->Np] * sgeo[surf_offset + NYID] +
                                 grad_T[vol_id + 2 * mesh->Np] * sgeo[surf_offset + NZID];

          integral += -k * normal_grad_T * sgeo[surf_offset + WSJID];
        }
      }
    }
  }

  freePointer(grad_T);

  // sum across all processes
  double total_integral;
  MPI_Allreduce(&integral, &total_integral, 1, MPI_DOUBLE, MPI_SUM, platform->comm.mpiComm());

  // multiply by the reference heat flux and an area factor to dimensionalize
  total_integral *= scales.flux_ref * scales.A_ref;

  return total_integral;
}

void
gradient(const int offset,
         const int e,
         const double * f,
         double * grad_f,
         const nek_mesh::NekMeshEnum pp_mesh)
{
  mesh_t * mesh = getMesh(pp_mesh);

  for (int k = 0; k < mesh->Nq; ++k)
  {
    for (int j = 0; j < mesh->Nq; ++j)
    {
      for (int i = 0; i < mesh->Nq; ++i)
      {
        const int gid = e * mesh->Np * mesh->Nvgeo + k * mesh->Nq * mesh->Nq + j * mesh->Nq + i;
        const double drdx = vgeo[gid + RXID * mesh->Np];
        const double drdy = vgeo[gid + RYID * mesh->Np];
        const double drdz = vgeo[gid + RZID * mesh->Np];
        const double dsdx = vgeo[gid + SXID * mesh->Np];
        const double dsdy = vgeo[gid + SYID * mesh->Np];
        const double dsdz = vgeo[gid + SZID * mesh->Np];
        const double dtdx = vgeo[gid + TXID * mesh->Np];
        const double dtdy = vgeo[gid + TYID * mesh->Np];
        const double dtdz = vgeo[gid + TZID * mesh->Np];

        // compute 'r' and 's' derivatives of (q_m) at node n
        double dpdr = 0.f, dpds = 0.f, dpdt = 0.f;

        for (int n = 0; n < mesh->Nq; ++n)
        {
          const double Dr = mesh->D[i * mesh->Nq + n];
          const double Ds = mesh->D[j * mesh->Nq + n];
          const double Dt = mesh->D[k * mesh->Nq + n];

          dpdr += Dr * f[e * mesh->Np + k * mesh->Nq * mesh->Nq + j * mesh->Nq + n];
          dpds += Ds * f[e * mesh->Np + k * mesh->Nq * mesh->Nq + n * mesh->Nq + i];
          dpdt += Dt * f[e * mesh->Np + n * mesh->Nq * mesh->Nq + j * mesh->Nq + i];
        }

        const int id = k * mesh->Nq * mesh->Nq + j * mesh->Nq + i;
        grad_f[id + 0 * offset] = drdx * dpdr + dsdx * dpds + dtdx * dpdt;
        grad_f[id + 1 * offset] = drdy * dpdr + dsdy * dpds + dtdy * dpdt;
        grad_f[id + 2 * offset] = drdz * dpdr + dsdz * dpds + dtdz * dpdt;
      }
    }
  }
}

bool
isHeatFluxBoundary(const int boundary)
{
  // the heat flux boundary is now named 'codedFixedGradient', but 'fixedGradient'
  // will be present for backwards compatibility

  // TODO
  //return (bcMap::text(boundary, "scalar00") == "fixedGradient") ||
  //       (bcMap::text(boundary, "scalar00") == "codedFixedGradient");
  return true;
}

bool
isMovingMeshBoundary(const int boundary)
{
  // TODO
  //return bcMap::text(boundary, "mesh") == "codedFixedValue";
  return true;
}

bool
isTemperatureBoundary(const int boundary)
{
  auto bcType = platform->app->bc->typeID(boundary, "scalar temperature");
  return bcType == bdryBase::bcType_zeroDirichlet || bcType == bdryBase::bcType_udfDirichlet;
}

const std::string
temperatureBoundaryType(const int boundary)
{
  // TODO
  //return bcMap::text(boundary, "scalar00");
  return true;
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
  MPI_Allreduce(&n_local, &n_global, 1, MPI_INT, MPI_SUM, platform->comm.mpiComm());
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
  if (hasCHT())
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

double
get_scalar01(const int id, const int surf_offset = 0)
{
  nrs_t * nrs = (nrs_t *)nrsPtr();
  return S[id + 1 * scalarFieldOffset()];
}

double
get_scalar02(const int id, const int surf_offset = 0)
{
  nrs_t * nrs = (nrs_t *)nrsPtr();
  return S[id + 2 * scalarFieldOffset()];
}

double
get_scalar03(const int id, const int surf_offset = 0)
{
  nrs_t * nrs = (nrs_t *)nrsPtr();
  return S[id + 3 * scalarFieldOffset()];
}

double
get_usrwrk00(const int id, const int surf_offset = 0)
{
  nrs_t * nrs = (nrs_t *)nrsPtr();
  return usrwrk[id];
}

double
get_usrwrk01(const int id, const int surf_offset = 0)
{
  nrs_t * nrs = (nrs_t *)nrsPtr();
  return usrwrk[id + nrs->fieldOffset];
}

double
get_usrwrk02(const int id, const int surf_offset = 0)
{
  nrs_t * nrs = (nrs_t *)nrsPtr();
  return usrwrk[id + 2 * nrs->fieldOffset];
}

double
get_temperature(const int id, const int surf_offset)
{
  nrs_t * nrs = (nrs_t *)nrsPtr();
  const auto sid = nrs->scalar->nameToIndex.find("temperature")->second;
  const auto offset = nrs->scalar->fieldOffset();
  std::vector<dfloat> temperature(S.begin() + sid * offset, S.begin() + (sid + 1) * offset);
  return temperature[id];
}

double
get_flux(const int id, const int surf_offset)
{
  // TODO: this function does not support non-constant thermal conductivity
  double k;
  platform->options.getArgs("SCALAR00 DIFFUSIVITY", k);

  nrs_t * nrs = (nrs_t *)nrsPtr();

  // this call of nek_mesh::all should be fine because flux is not a 'field' which can be
  // provided to the postprocessors which have the option to operate only on part of the mesh
  auto mesh = getMesh(nek_mesh::all);
  int elem_id = id / mesh->Np;
  int vertex_id = id % mesh->Np;

  const auto sid = nrs->scalar->nameToIndex.find("temperature")->second;
  const auto offset = nrs->scalar->fieldOffset();
  std::vector<dfloat> temperature(S.begin() + sid * offset, S.begin() + (sid + 1) * offset);
  // This function is slightly inefficient, because we compute grad(T) for all nodes in
  // an element even though we only call this function for one node at a time
  double * grad_T = (double *)calloc(3 * mesh->Np, sizeof(double));
  gradient(mesh->Np, elem_id, temperature.data(), grad_T, nek_mesh::all);

  double normal_grad_T = grad_T[vertex_id + 0 * mesh->Np] * sgeo[surf_offset + NXID] +
                         grad_T[vertex_id + 1 * mesh->Np] * sgeo[surf_offset + NYID] +
                         grad_T[vertex_id + 2 * mesh->Np] * sgeo[surf_offset + NZID];
  freePointer(grad_T);

  return -k * normal_grad_T;
}

double
get_pressure(const int id, const int surf_offset)
{
  nrs_t * nrs = (nrs_t *)nrsPtr();
  return P[id];
}

double
get_unity(const int /* id */, const int surf_offset)
{
  return 1.0;
}

double
get_velocity_x(const int id, const int surf_offset)
{
  nrs_t * nrs = (nrs_t *)nrsPtr();
  return U[id + 0 * nrs->fieldOffset];
}

double
get_velocity_y(const int id, const int surf_offset)
{
  nrs_t * nrs = (nrs_t *)nrsPtr();
  return U[id + 1 * nrs->fieldOffset];
}

double
get_velocity_z(const int id, const int surf_offset)
{
  nrs_t * nrs = (nrs_t *)nrsPtr();
  return U[id + 2 * nrs->fieldOffset];
}

double
get_velocity(const int id, const int surf_offset)
{
  nrs_t * nrs = (nrs_t *)nrsPtr();
  int offset = nrs->fieldOffset;

  return std::sqrt(U[id + 0 * offset] * U[id + 0 * offset] +
                   U[id + 1 * offset] * U[id + 1 * offset] +
                   U[id + 2 * offset] * U[id + 2 * offset]);
}

double
get_velocity_x_squared(const int id, const int surf_offset)
{
  return std::pow(get_velocity_x(id, surf_offset), 2);
}

double
get_velocity_y_squared(const int id, const int surf_offset)
{
  return std::pow(get_velocity_y(id, surf_offset), 2);
}

double
get_velocity_z_squared(const int id, const int surf_offset)
{
  return std::pow(get_velocity_z(id, surf_offset), 2);
}

void
checkFieldValidity(const field::NekWriteEnum & field)
{
  switch (field)
  {
    case field::flux:
      if (!hasTemperatureVariable())
        mooseError("Cannot get NekRS heat flux "
                   "because your Nek case files do not have a temperature variable!");
      break;
    case field::heat_source:
      if (!hasTemperatureVariable())
        mooseError("Cannot get NekRS heat source "
                   "because your Nek case files do not have a temperature variable!");
      break;
    case field::x_displacement:
    case field::y_displacement:
    case field::z_displacement:
    case field::mesh_velocity_x:
    case field::mesh_velocity_y:
    case field::mesh_velocity_z:
      break;
    default:
      mooseError("Unhandled NekWriteEnum in checkFieldValidity!");
  }
}

void
checkFieldValidity(const field::NekFieldEnum & field)
{
  // by placing this check here, as opposed to inside the NekFieldInterface,
  // we can also leverage this error checking for the 'outputs' of NekRSProblem,
  // which does not inherit from NekFieldInterface but still accesses the solutionPointers.
  // If this gets moved elsewhere, need to be sure to add dedicated testing for
  // the 'outputs' on NekRSProblem.

  // TODO: would be nice for NekRSProblem to only access field information via the
  // NekFieldInterface; refactor later

  switch (field)
  {
    case field::temperature:
      if (!hasTemperatureVariable())
        mooseError("Cannot find 'temperature' "
                   "because your Nek case files do not have a temperature variable!");
      break;
    case field::scalar01:
      if (!hasScalarVariable(1))
        mooseError("Cannot find 'scalar01' "
                   "because your Nek case files do not have a scalar01 variable!");
      break;
    case field::scalar02:
      if (!hasScalarVariable(2))
        mooseError("Cannot find 'scalar02' "
                   "because your Nek case files do not have a scalar02 variable!");
      break;
    case field::scalar03:
      if (!hasScalarVariable(3))
        mooseError("Cannot find 'scalar03' "
                   "because your Nek case files do not have a scalar03 variable!");
      break;
    case field::usrwrk00:
      if (n_usrwrk_slots < 1)
        mooseError("Cannot find 'usrwrk00' because you have only allocated 'n_usrwrk_slots = " +
                   std::to_string(n_usrwrk_slots) + "'");
      break;
    case field::usrwrk01:
      if (n_usrwrk_slots < 2)
        mooseError("Cannot find 'usrwrk01' because you have only allocated 'n_usrwrk_slots = " +
                   std::to_string(n_usrwrk_slots) + "'");
      break;
    case field::usrwrk02:
      if (n_usrwrk_slots < 3)
        mooseError("Cannot find 'usrwrk02' because you have only allocated 'n_usrwrk_slots = " +
                   std::to_string(n_usrwrk_slots) + "'");
      break;
  }
}

double (*solutionPointer(const field::NekWriteEnum & field))(int, int)
{
  double (*f)(int, int);

  checkFieldValidity(field);

  switch (field)
  {
    case field::flux:
      f = &get_flux;
      break;
    default:
      mooseError("Unhandled NekWriteEnum in solutionPointer!");
  }

  return f;
}

double (*solutionPointer(const field::NekFieldEnum & field))(int, int)
{
  // we include this here as well, in addition to within the NekFieldInterface, because
  // the NekRSProblem accesses these methods without inheriting from NekFieldInterface
  checkFieldValidity(field);

  double (*f)(int, int);

  switch (field)
  {
    case field::velocity_x:
      f = &get_velocity_x;
      break;
    case field::velocity_y:
      f = &get_velocity_y;
      break;
    case field::velocity_z:
      f = &get_velocity_z;
      break;
    case field::velocity:
      f = &get_velocity;
      break;
    case field::velocity_component:
      mooseError("The 'velocity_component' field is not compatible with the solutionPointer "
                 "interface!");
      break;
    case field::velocity_x_squared:
      f = &get_velocity_x_squared;
      break;
    case field::velocity_y_squared:
      f = &get_velocity_y_squared;
      break;
    case field::velocity_z_squared:
      f = &get_velocity_z_squared;
      break;
    case field::temperature:
      f = &get_temperature;
      break;
    case field::pressure:
      f = &get_pressure;
      break;
    case field::scalar01:
      f = &get_scalar01;
      break;
    case field::scalar02:
      f = &get_scalar02;
      break;
    case field::scalar03:
      f = &get_scalar03;
      break;
    case field::unity:
      f = &get_unity;
      break;
    case field::usrwrk00:
      f = &get_usrwrk00;
      break;
    case field::usrwrk01:
      f = &get_usrwrk01;
      break;
    case field::usrwrk02:
      f = &get_usrwrk02;
      break;
    default:
      throw std::runtime_error("Unhandled 'NekFieldEnum'!");
  }

  return f;
}

void
initializeDimensionalScales(const double U,
                            const double T,
                            const double dT,
                            const double L,
                            const double rho,
                            const double Cp,
                            const double s01,
                            const double ds01,
                            const double s02,
                            const double ds02,
                            const double s03,
                            const double ds03)
{
  scales.U_ref = U;
  scales.T_ref = T;
  scales.dT_ref = dT;
  scales.L_ref = L;
  scales.A_ref = L * L;
  scales.V_ref = L * L * L;
  scales.rho_ref = rho;
  scales.Cp_ref = Cp;
  scales.t_ref = L / U;
  scales.P_ref = rho * U * U;

  scales.s01_ref = s01;
  scales.ds01_ref = ds01;
  scales.s02_ref = s02;
  scales.ds02_ref = ds02;
  scales.s03_ref = s03;
  scales.ds03_ref = ds03;

  scales.flux_ref = rho * U * Cp * dT;
  scales.source_ref = scales.flux_ref / L;
}

double
referenceLength()
{
  return scales.L_ref;
}

double
referenceTime()
{
  return scales.t_ref;
}

double
referenceArea()
{
  return scales.A_ref;
}

double
referenceVolume()
{
  return scales.V_ref;
}

Real
nondimensionalAdditive(const field::NekFieldEnum & field)
{
  switch (field)
  {
    case field::temperature:
      return scales.T_ref;
    case field::scalar01:
      return scales.s01_ref;
    case field::scalar02:
      return scales.s02_ref;
    case field::scalar03:
      return scales.s03_ref;
    default:
      return 0;
  }
}

Real
nondimensionalAdditive(const field::NekWriteEnum & field)
{
  switch (field)
  {
    case field::flux:
    case field::heat_source:
    case field::x_displacement:
    case field::y_displacement:
    case field::z_displacement:
    case field::mesh_velocity_x:
    case field::mesh_velocity_y:
    case field::mesh_velocity_z:
      return 0.0;
    default:
      mooseError("Unhandled NekWriteEnum in nondimensionalAdditive!");
  }
}

Real
nondimensionalDivisor(const field::NekWriteEnum & field)
{
  switch (field)
  {
    case field::flux:
      return scales.flux_ref;
    case field::heat_source:
      return scales.source_ref;
    case field::x_displacement:
    case field::y_displacement:
    case field::z_displacement:
      return scales.L_ref;
    case field::mesh_velocity_x:
    case field::mesh_velocity_y:
    case field::mesh_velocity_z:
      return scales.U_ref;
    default:
      mooseError("Unhandled NekWriteEnum in nondimensionalDivisor!");
  }
}

Real
nondimensionalDivisor(const field::NekFieldEnum & field)
{
  switch (field)
  {
    case field::velocity_x:
    case field::velocity_y:
    case field::velocity_z:
    case field::velocity:
    case field::velocity_component:
      return scales.U_ref;
    case field::velocity_x_squared:
    case field::velocity_y_squared:
    case field::velocity_z_squared:
      return scales.U_ref * scales.U_ref;
    case field::temperature:
      return scales.dT_ref;
    case field::pressure:
      return scales.P_ref;
    case field::scalar01:
      return scales.ds01_ref;
    case field::scalar02:
      return scales.ds02_ref;
    case field::scalar03:
      return scales.ds03_ref;
    case field::unity:
      // no dimensionalization needed
      return 1.0;
    case field::usrwrk00:
      return scratchUnits(0);
    case field::usrwrk01:
      return scratchUnits(1);
    case field::usrwrk02:
      return scratchUnits(2);
    default:
      throw std::runtime_error("Unhandled 'NekFieldEnum'!");
  }
}

Real
scratchUnits(const int slot)
{
  // if (indices.flux != -1 && slot == indices.flux / nekrs::fieldOffset())
  //   return scales.flux_ref;
  // else if (indices.heat_source != -1 && slot == indices.heat_source / nekrs::fieldOffset())
  //   return scales.source_ref;
  if (is_nondimensional)
  {
    mooseDoOnce(mooseWarning(
        "The units of 'usrwrk0" + std::to_string(slot) +
        "' are unknown, so we cannot dimensionalize any objects using 'field = usrwrk0" +
        std::to_string(slot) +
        "'. The output for this quantity will be given in non-dimensional form.\n\nYou will need "
        "to manipulate the data manually from Cardinal if you need to dimensionalize it."));
  }

  return 1.0;
}

void
nondimensional(const bool n)
{
  is_nondimensional = n;
}

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

void
initializeNekHostArrays()
{
  nrs_t * nrs = (nrs_t *)nrsPtr();
  mesh_t * mesh = entireMesh();

  x.resize(mesh->Nlocal);
  y.resize(mesh->Nlocal);
  z.resize(mesh->Nlocal);

  U.resize(mesh->dim * nrs->fluid->fieldOffset);
  P.resize(mesh->Nlocal);
  S.resize(nrs->scalar->NSfields * nrs->scalar->fieldOffset()); //offset is same for all scalars
}

} // end namespace nekrs

#endif
