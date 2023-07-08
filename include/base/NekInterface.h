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

#pragma once

#include "CardinalEnums.h"
#include "MooseTypes.h"
#include "NekBoundaryCoupling.h"
#include "NekVolumeCoupling.h"
#include "nekrs.hpp"
#include "bcMap.hpp"
#include "udf.hpp"
#include "libmesh/point.h"
#include "mesh.h"
#include <string>
#include <vector>

/**
 * \brief Cardinal-specific nekRS API
 *
 * nekRS ships with a rudimentary API in their nekrs namespace, but we need additional
 * functionality from within Cardinal. Many of these functions are quite basic and could
 * eventually be ported back into nekRS itself.
 */
namespace nekrs
{

static int build_only;

/// Allocate memory for the host mesh parameters
void initializeHostMeshParameters();

/// Update the mesh parameters on host
void updateHostMeshParameters();

dfloat * getSgeo();
dfloat * getVgeo();

/**
 * Set the absolute tolerance for checking energy conservation in data transfers to Nek
 * @param[in] tol tolerance
 */
void setAbsoluteTol(double tol);

/**
 * Set the relative tolerance for checking energy conservation in data transfers to Nek
 * @param[in] tol tolerance
 */
void setRelativeTol(double tol);

/**
 * Nek's runtime statistics are formed by collecting a timer of both the initialization
 * and accumulated run time. We unfortunately have to split this across multiple classes,
 * so if we want correct times we need to have NekInitAction save the value of the time
 * spent on initialization.
 * @param[in] time time spent on initialization
 */
void setNekSetupTime(const double & time);

/**
 * Get time spent on initialization
 * @return time spent on initialization
 */
double getNekSetupTime();

/**
 * Set the start time used by NekRS
 * @param[in] start start time
 */
void setStartTime(const double & start);

/**
 * Whether NekRS itself has been initialized yet
 * @return whether NekRS is initialized
 */
bool isInitialized();

/**
 * Write a field file containing a specific slot of the nrs->usrwrk scratch space;
 * this will write the field to the 'temperature' slot in a field file.
 * @param[in] slot index in the nrs->usrwrk array to write
 * @param[in] prefix prefix for file name
 * @param[in] time simulation time to write file for
 * @param[in] step time step index
 * @param[in] write_coords whether to write the mesh coordinates
 */
void write_usrwrk_field_file(const int & slot, const std::string & prefix, const dfloat & time, const int & step, const bool & write_coords);

/**
 * Write a field file containing pressure, velocity, and scalars with given prefix
 * @param[in] prefix three-character prefix
 * @param[in] time time
 * @param[in] step time step index
 */
void write_field_file(const std::string & prefix, const dfloat time, const int & step);

/**
 * Indicate whether NekRS was run in build-only mode (this doesn't actually
 * cause NekRS to run in build-only mode, but only provides an interface to
 * this information elsewhere).
 * @param[in] buildOnly whether NekRS is to be run in build-only mode
 */
void buildOnly(int buildOnly);

/**
 * Whether NekRS was run in JIT build-only mode
 * @return whether NekRS was run in build-only mode
 */
int buildOnly();

/**
 * Interpolate a volume between NekRS's GLL points and a given-order receiving/sending mesh
 */
void interpolateVolumeHex3D(const double * I, double * x, int N, double * Ix, int M);

/**
 * Whether nekRS's input file has CHT
 * @return whether nekRS input files model CHT
 */
bool hasCHT();

/**
 * Whether nekRS's input file indicates a moving mesh
 * @return whether nekRS's input file indicates a moving mesh
 */
bool hasMovingMesh();

/**
 * Whether nekRS's input file indicates a variable time stepping scheme
 * @return whether nekRS's input file indicates a variable time stepping
 */
bool hasVariableDt();

/**
 * Whether nekRS's input file has the blending mesh solver
 * @return whether nekRS's input file has a non-user [MESH] solver
 */
bool hasBlendingSolver();

/**
 * Whether nekRS's input file has the user mesh solver
 * @return whether nekRS's input file has [MESH] solver = user
 */
bool hasUserMeshSolver();

/**
 * Whether nekRS's input file intends to terminate the simulation based on a wall time
 * @return whether a wall time is used in nekRS to end the simulation
 */
bool endControlElapsedTime();

/**
 * Whether nekRS's input file intends to terminate the simulation based on an end time
 * @return whether an end time is used in nekRS to end the simulation
 */
bool endControlTime();

/**
 * Whether nekRS's input file intends to terminate the simulation based on a number of steps
 * @return whether a time step interval is used in nekRS to end the simulation
 */
bool endControlNumSteps();

/**
 * Offset increment for indexing into multi-volume arrays for the scalar fields.
 * This assumes that all scalars are the same length as the temperature scalar.
 * TODO: evaluate whether this works if nekRS uses CHT
 * @return scalar field offset
 */
int scalarFieldOffset();

/**
 * Offset increment for indexing into the velocity array
 * @return velocity field offset
 */
int velocityFieldOffset();

/**
 * Get the "entire" NekRS mesh. For cases with a temperature scalar, this returns
 * nrs->meshT, which will cover both the fluid and solid regions if CHT is present.
 * For flow-only cases, this will return the flow mesh.
 * @return entire NekRS mesh
 */
mesh_t * entireMesh();

/**
 * Get the mesh for the flow solve
 * @return flow mesh
 */
mesh_t * flowMesh();

/**
 * Get the mesh for the temperature scalar
 * @return temperature mesh
 */
mesh_t * temperatureMesh();

/**
 * Get the mesh to act on
 * @param[in] pp_mesh which NekRS mesh to operate on
 * @return mesh to act on
 */
mesh_t * getMesh(const nek_mesh::NekMeshEnum pp_mesh);

/**
 * Get the process rank
 * @return process rank
 */
int commRank();

/**
 * Get the communicator size
 * @return communicator size
 */
int commSize();

/**
 * Whether nekRS's input file indicates that the problem has a temperature variable
 * @return whether the nekRS problem includes a temperature variable
 */
bool hasTemperatureVariable();

/**
 * Whether nekRS actually solves for temperature (as opposed to setting its solver to 'none')
 * @return whether nekRS will solve for temperature
 */
bool hasTemperatureSolve();

/**
 * Whether nekRS's input file indicates that the problem has a scalar0(scalarId) variable
 * @param[in] scalarId scalar number, i.e. for scalar03 scalarId=3
 * @return whether the nekRS problem includes the scalar0(scalarId) variable
 */
bool hasScalarVariable(int scalarId);

/**
 * Whether nekRS contains an OCCA kernel to apply a source to the passive scalar equations
 * @return whether nekRS has an OCCA kernel for apply a passive scalar source
 */
bool hasHeatSourceKernel();

/**
 * Whether the scratch space has already been allocated by the user
 * @return whether scratch space is already allocated
 */
bool scratchAvailable();

/**
 * Initialize scratch space for data to get sent into NekRS
 * @param[in] n_slots number of slots (for volume arrays) to allocate
 */
void initializeScratch(const unsigned int & n_slots);

/// Free the scratch space
void freeScratch();

/**
 * Get the viscosity used in the definition of the Reynolds number; note that
 * for dimensional cases, this is only guaranteed to be correct if the viscosity is constant.
 * @return constant dynamic viscosity
 */
double viscosity();

/**
 * Get the Prandtl number; note that for dimensional cases, this is only guaranteed
 * to be correct if the density, viscosity, heat capacity, and conductivity are constant.
 * @return constant Prandtl number
 */
double Pr();

/// Copy the deformation from host to device
void copyDeformationToDevice();

template <typename T>
void allgatherv(const std::vector<int> & base_counts,
                const T * input,
                T * output,
                const int multiplier = 1);

/**
 * Determine the receiving counts and displacements for all gather routines
 * @param[in] base_counts unit-wise receiving counts for each process
 * @param[out] counts receiving counts from each process
 * @param[out] displacement displacement for each process's counts
 * @param[in] multiplier optional multiplier on the face-based data
 */
void displacementAndCounts(const std::vector<int> & base_counts,
                           int * counts,
                           int * displacement,
                           const int multiplier);

/**
 * Form the 2-D interpolation matrix from a starting GLL quadrature rule to an ending
 * GLL quadrature rule.
 * @param[out] I interpolation matrix
 * @param[in] starting_points number of points in the source quadrature rule
 * @param[in] ending_points number of points in the end quadrature rule
 */
void interpolationMatrix(double * I, int starting_points, int ending_points);

/**
 * Interpolate face data onto a new set of points
 * @param[in] scratch available scratch space for the calculation
 * @param[in] I interpolation matrix
 * @param[in] x face data to be interpolated
 * @param[in] N number of points in 1-D to be interpolated
 * @param[out] Ix interpolated data
 * @param[in] M resulting number of interpolated points in 1-D
 */
void interpolateSurfaceFaceHex3D(
    double * scratch, const double * I, double * x, int N, double * Ix, int M);

/**
 * Compute the face centroid given a local element ID and face ID (NOTE: returns in dimensional
 * form)
 * @param[in] local_elem_id local element ID on this rank
 * @param[in] local_face_id local face ID on the element
 * @return centroid
 */
Point centroidFace(int local_elem_id, int local_face_id);

/**
 * Compute the centroid given a local element ID (NOTE: returns in dimensional form)
 * @param[in] local_elem_id local element ID on this rank
 * @return centroid
 */
Point centroid(int local_elem_id);

/**
 * Get the coordinate given a local element ID and local node ID (NOTE: returns in dimensional form)
 * @param[in] local_elem_id local element ID on this rank
 * @param[in] local_node_id local node ID on this element
 * @return point
 */
Point gllPoint(int local_elem_id, int local_node_id);

/**
 * Get the coordinate given a local element ID, a local face ID, and local node ID (NOTE: returns in
 * dimensional form)
 * @param[in] local_elem_id local element ID on this rank
 * @param[in] local_face_id local face ID on this element
 * @param[in] local_node_id local node ID on this element
 * @return point
 */
Point gllPointFace(int local_elem_id, int local_face_id, int local_node_id);

/**
 * Integrate the scratch space over boundaries
 * @param[in] slot slot in scratch space
 * @param[in] boundary boundaries over which to integrate the scratch space
 * @param[in] pp_mesh portion of NekRS mesh to integrate over
 * @return boundary integrated scratch space, with one value per sideset
 */
std::vector<double> usrwrkSideIntegral(const unsigned int & slot, const std::vector<int> & boundary,
  const nek_mesh::NekMeshEnum pp_mesh);

/**
 * Volume integrate the scratch space
 * @param[in] slot slot in scratch space to i ntegrat
 * @param[in] pp_mesh NekRS mesh to integrate over
 * @return volume integrated scratch space
 */
double usrwrkVolumeIntegral(const unsigned int & slot, const nek_mesh::NekMeshEnum pp_mesh);

/**
 * Scale a slot in the usrwrk by a fixed value (multiplication)
 * @param[in] slot slot in usrwrk to modify
 * @param[in] value value to multiply on scratch slot
 */
void scaleUsrwrk(const unsigned int & slot, const dfloat & value);

/**
 * Normalize the flux sent to nekRS to conserve the total flux
 * @param[in] nek_boundary_coupling data structure holding boundary coupling info
 * @param[in] boundary boundaries for which to normalize the flux
 * @param[in] moose_integral total integrated flux from MOOSE to conserve
 * @param[in] nek_integral total integrated flux in nekRS to adjust
 * @param[out] normalized_nek_integral final normalized nek flux integral
 * @return whether normalization was successful, i.e. normalized_nek_integral equals moose_integral
 */
bool normalizeFluxBySideset(const NekBoundaryCoupling & nek_boundary_coupling,
                   const std::vector<int> & boundary,
                   const std::vector<double> & moose_integral,
                   std::vector<double> & nek_integral,
                   double & normalized_nek_integral);

/**
 * Normalize the flux sent to nekRS to conserve the total flux
 * @param[in] nek_boundary_coupling data structure holding boundary coupling info
 * @param[in] boundary boundaries for which to normalize the flux
 * @param[in] moose_integral total integrated flux from MOOSE to conserve
 * @param[in] nek_integral total integrated flux in nekRS to adjust
 * @param[out] normalized_nek_integral final normalized nek flux integral
 * @return whether normalization was successful, i.e. normalized_nek_integral equals moose_integral
 */
bool normalizeFlux(const NekBoundaryCoupling & nek_boundary_coupling,
                   const std::vector<int> & boundary,
                   const double moose_integral,
                   double nek_integral,
                   double & normalized_nek_integral);

/**
 * Compute the area of a set of boundary IDs
 * @param[in] boundary_id nekRS boundary IDs for which to perform the integral
 * @param[in] pp_mesh which NekRS mesh to operate on
 * @return area integral
 */
double area(const std::vector<int> & boundary_id, const nek_mesh::NekMeshEnum pp_mesh);

/**
 * Compute the area integral of a given integrand over a set of boundary IDs
 * @param[in] boundary_id nekRS boundary IDs for which to perform the integral
 * @param[in] integrand field to integrate
 * @param[in] pp_mesh which NekRS mesh to operate on
 * @return area integral of a field
 */
double sideIntegral(const std::vector<int> & boundary_id, const field::NekFieldEnum & integrand,
                    const nek_mesh::NekMeshEnum pp_mesh);

/**
 * Compute the volume over the entire scalar mesh
 * @param[in] pp_mesh which NekRS mesh to operate on
 * @return volume integral
 */
double volume(const nek_mesh::NekMeshEnum pp_mesh);

/**
 * Dimensionalize a volume
 * @param[in] integral integral to dimensionalize
 */
void dimensionalizeVolume(double & integral);

/**
 * Dimensionalize an area
 * @param[in] integral integral to dimensionalize
 */
void dimensionalizeArea(double & integral);

/**
 * Dimensionalize a given integral of f over volume, i.e. fdV
 * @param[in] integrand field to dimensionalize
 * @param[in] volume volume of the domain (only used for dimensionalizing temperature)
 * @param[in] integral integral to dimensionalize
 */
void dimensionalizeVolumeIntegral(const field::NekFieldEnum & integrand,
                                  const Real & volume,
                                  double & integral);

/**
 * Dimensionalize a given integral of f over a side, i.e. fdS
 * @param[in] integrand field to dimensionalize
 * @param[in] area area of the boundary
 * @param[in] integral integral to dimensionalize
 */
void dimensionalizeSideIntegral(const field::NekFieldEnum & integrand,
                                const Real & area,
                                double & integral);

/**
 * Dimensionalize a given integral of f over a side, i.e. fdS
 * @param[in] integrand field to dimensionalize
 * @param[in] boundary_id boundary IDs for the integral
 * @param[in] integral integral to dimensionalize
 * @param[in] pp_mesh which NekRS mesh to operate on
 */
void dimensionalizeSideIntegral(const field::NekFieldEnum & integrand,
                                const std::vector<int> & boundary_id,
                                double & integral,
                                const nek_mesh::NekMeshEnum pp_mesh);

/**
 * Compute the volume integral of a given integrand over the entire scalar mesh
 * @param[in] integrand field to integrate
 * @param[in] volume volume of the domain (only used for dimensionalizing temperature)
 * @param[in] pp_mesh which NekRS mesh to operate on
 * @return volume integral of a field
 */
double volumeIntegral(const field::NekFieldEnum & integrand,
                      const double & volume,
                      const nek_mesh::NekMeshEnum pp_mesh);

/**
 * Compute the mass flowrate over a set of boundary IDs
 * @param[in] boundary_id nekRS boundary IDs for which to compute the mass flowrate
 * @param[in] pp_mesh which NekRS mesh to operate on
 * @return mass flowrate
 */
double massFlowrate(const std::vector<int> & boundary_id,
                    const nek_mesh::NekMeshEnum pp_mesh);

/**
 * Compute the mass flux weighted integral of a given integrand over a set of boundary IDs
 * @param[in] boundary_id nekRS boundary IDs for which to perform the integral
 * @param[in] integrand field to integrate and weight by mass flux
 * @param[in] pp_mesh which NekRS mesh to operate on
 * @return mass flux weighted area average of a field
 */
double sideMassFluxWeightedIntegral(const std::vector<int> & boundary_id,
                                    const field::NekFieldEnum & integrand,
                                    const nek_mesh::NekMeshEnum pp_mesh);

/**
 * Compute the integral of pressure on a surface, multiplied by the unit normal
 * of the surface with a specified direction vector.
 * @param[in] boundary_id NekRS boundary IDs for which to perform the integral
 * @param[in] direction unit vector to dot with the boundary surface normal
 * @param[in] pp_mesh which NekRS mesh to operate on
 * @return pressure surface force, along a particular direction
 */
double pressureSurfaceForce(const std::vector<int> & boundary_id, const Point & direction, const nek_mesh::NekMeshEnum pp_mesh);

/**
 * Compute the heat flux over a set of boundary IDs
 * @param[in] boundary_id nekRS boundary IDs for which to perform the integral
 * @param[in] pp_mesh which NekRS mesh to operate on
 * @return heat flux area integral
 */
double heatFluxIntegral(const std::vector<int> & boundary_id,
                        const nek_mesh::NekMeshEnum pp_mesh);

/**
 * Limit the temperature in nekRS to within the range of [min_T, max_T]
 * @param[in] min_T minimum temperature allowable in nekRS
 * @param[in] max_T maximum temperature allowable in nekRS
 */
void limitTemperature(const double * min_T, const double * max_T);

/**
 * Compute the gradient of a volume field
 * @param[in] offset in the gradient field for each component (grad_x, grad_y, or grad_z)
 * @param[in] f field to compute the gradient of
 * @param[in] pp_mesh which NekRS mesh to operate on
 * @param[out] grad_f gradient of field
 */
void gradient(const int offset, const double * f, double * grad_f,
              const nek_mesh::NekMeshEnum pp_mesh);

/**
 * Find the extreme value of a given field over the entire nekRS domain
 * @param[in] field field to find the minimum value of
 * @param[in] pp_mesh which NekRS mesh to operate on
 * @param[in] max whether to take the maximum (or if false, the minimum)
 * @return max or min value of field in volume
 */
double volumeExtremeValue(const field::NekFieldEnum & field,
                          const nek_mesh::NekMeshEnum pp_mesh,
                          const bool max);

/**
 * Find the extreme of a given field over a set of boundary IDs
 * @param[in] boundary_id nekRS boundary IDs for which to find the extreme value
 * @param[in] field field to find the maximum value of
 * @param[in] pp_mesh which NekRS mesh to operate on
 * @param[in] max whether to take the maximum (or if false, the minimum)
 * @return max or min value of field on boundary
 */
double sideExtremeValue(const std::vector<int> & boundary_id, const field::NekFieldEnum & field,
                        const nek_mesh::NekMeshEnum pp_mesh, const bool max);

/**
 * Number of faces per element; because NekRS only supports HEX20, this should be 6
 * @return number of faces per mesh element
 */
int Nfaces();

/**
 * Whether the specific boundary is a flux boundary
 * @param[in] boundary boundary ID
 * @return whether boundary is a flux boundary
 */
bool isHeatFluxBoundary(const int boundary);

/**
 * Whether the specific boundary is a moving mesh boundary
 * @param[in] boundary boundary ID
 * @return whether boundary is a moving mesh boundary
 */
bool isMovingMeshBoundary(const int boundary);

/**
 * Whether the specific boundary is a specified temperature boundary
 * @param[in] boundary boundary ID
 * @return whether boundary is a temperature boundary
 */
bool isTemperatureBoundary(const int boundary);

/**
 * String name indicating the temperature boundary condition type on a given boundary
 * @param[in] boundary boundary ID
 * @return string name of boundary condition type
 */
const std::string temperatureBoundaryType(const int boundary);

/**
 * Polynomial order used in nekRS solution
 * @return polynomial order
 */
int polynomialOrder();

/**
 * Total number of volume elements in nekRS mesh summed over all processes
 * @return number of volume elements
 */
int Nelements();

/**
 * Mesh dimension
 * @return mesh dimension
 */
int dim();

/**
 * \brief Number of vertices required to define an element face
 * Vertices refer to the points required to place the "corners" of an element face,
 * and _not_ the quadrature points. For instance, for hexahedral elements, the number of vertices
 * per face is 4 regardless of the polynomial order.
 * @return Number of vertices per element face
 */
int NfaceVertices();

/**
 * Total number of element faces on a boundary of the nekRS mesh summed over all processes
 * @return number of boundary element faces
 */
int NboundaryFaces();

/**
 * Number of boundary IDs in the nekRS mesh
 * @return number of boundary IDs
 */
int NboundaryID();

/**
 * Whether the provided boundary IDs are all valid in the nekRS mesh
 * @param[in] boundary_id vector of boundary IDs to check
 * @param[out] first_invalid_id first invalid ID encountered for printing an error on the MOOSE side
 * @param[out] n_boundaries maximum valid boundary ID for printing an error on the MOOSE side
 * @return whether all boundaries are valid
 */
bool
validBoundaryIDs(const std::vector<int> & boundary_id, int & first_invalid_id, int & n_boundaries);

/**
 * Store the rank-local element, element-local face, and rank ownership for boundary coupling
 * @param[in] boundary_id boundaries through which nekRS will be coupled
 * @param[out] N total number of surface elements
 */
void storeBoundaryCoupling(const std::vector<int> & boundary_id, int & N);

/**
 * Integer indices in the usrwrk scratch space for writing solutions from MOOSE.
 * These will be set from Cardinal. Not all will be used simultaneously.
 */
struct usrwrkIndices
{
  /// boundary heat flux (for conjugate heat transfer)
  int flux;

  /// volumetric heat source (for volumetric heating)
  int heat_source;

  /// x-velocity of moving boundary (for mesh blending solver)
  int mesh_velocity_x;

  /// y-velocity of moving boundary (for mesh blending solver)
  int mesh_velocity_y;

  /// z-velocity of moving boundary (for mesh blending solver)
  int mesh_velocity_z;

  /// boundary velocity (for separate domain coupling)
  int boundary_velocity;

  /// boundary temperature (for separate domain coupling)
  int boundary_temperature;

  /// boundary scalar01 (for separate domain coupling)
  int boundary_scalar01;

  /// boundary scalar02 (for separate domain coupling)
  int boundary_scalar02;

  /// boundary scalar03 (for separate domain coupling)
  int boundary_scalar03;
};

/// Characteristic scales assumed in nekRS if using a non-dimensional solution
struct characteristicScales
{
  double U_ref;

  double T_ref;

  double dT_ref;

  double L_ref;

  double A_ref;

  double V_ref;

  double rho_ref;

  double Cp_ref;

  double flux_ref;

  double source_ref;

  bool nondimensional_T;
};

/**
 * Get pointer to various solution functions (for reading only) based on enumeration
 * @param[in] field field to return a pointer to
 * @return function pointer to method that returns said field as a function of GLL index
 */
double (*solutionPointer(const field::NekFieldEnum & field))(int);

/**
 * Write various solution functions based on enumeration
 * @param[in] field field to write
 */
void (*solutionPointer(const field::NekWriteEnum & field))(int, dfloat);

/**
 * \brief Get the scalar01 solution at given GLL index
 *
 * @param[in] id GLL index
 * @return scalar01 value at index
 */
double scalar01(const int id);

/**
 * \brief Get the scalar02 solution at given GLL index
 *
 * @param[in] id GLL index
 * @return scalar02 value at index
 */
double scalar02(const int id);

/**
 * \brief Get the scalar03 solution at given GLL index
 *
 * @param[in] id GLL index
 * @return scalar03 value at index
 */
double scalar03(const int id);

/**
 * \brief Get the temperature solution at given GLL index
 *
 * Because nekRS stores all the passive scalars together in one flat array, this routine
 * simply indices into the entire passive scalar solution. In order to get temperature, you should
 * only index up to nrs->cds->fieldOffset.
 * @param[in] id GLL index
 * @return temperature value at index
 */
double temperature(const int id);

/**
 * Get the pressure solution at given GLL index
 * @param[in] id GLL index
 * @return pressure value at index
 */
double pressure(const int id);

/**
 * Return unity, for cases where the integrand or operator we are generalizing acts on 1
 * @param[in] id GLL index
 * @return unity
 */
double unity(const int id);

/**
 * Get the x-velocity at given GLL index
 * @param[in] id GLL index
 * @return x-velocity at index
 */
double velocity_x(const int id);

/**
 * Get the y-velocity at given GLL index
 * @param[in] id GLL index
 * @return y-velocity at index
 */
double velocity_y(const int id);

/**
 * Get the z-velocity at given GLL index
 * @param[in] id GLL index
 * @return z-velocity at index
 */
double velocity_z(const int id);

/**
 * Get the magnitude of the velocity solution at given GLL index
 * @param[in] id GLL index
 * @return velocity magnitude at index
 */
double velocity(const int id);

/**
 * Write a value into the user scratch space that holds the flux
 * @param[in] id index
 * @param[in] value value to write
 */
void flux(const int id, const dfloat value);

/**
 * Write a value into the user scratch space that holds the volumetric heat source
 * @param[in] id index
 * @param[in] value value to write
 */
void heat_source(const int id, const dfloat value);

/**
 * Write a value into the x-displacement
 * @param[in] id index
 * @param[in] value value to write
 */
void x_displacement(const int id, const dfloat value);

/**
 * Write a value into the y-displacement
 * @param[in] id index
 * @param[in] value value to write
 */
void y_displacement(const int id, const dfloat value);

/**
 * Write a value into the z-displacement
 * @param[in] id index
 * @param[in] value value to write
 */
void z_displacement(const int id, const dfloat value);

/**
 * Initialize the characteristic scales for a nondimesional solution
 * @param[in] U_ref reference velocity
 * @param[in] T_ref reference temperature
 * @param[in] dT_ref reference temperature range
 * @param[in] L_ref reference length scale
 * @param[in] rho_ref reference density
 * @param[in] Cp_ref reference heat capacity
 */
void initializeDimensionalScales(const double U_ref,
                                 const double T_ref,
                                 const double dT_ref,
                                 const double L_ref,
                                 const double rho_ref,
                                 const double Cp_ref);

/**
 * \brief Dimensionalize a field by multiplying the nondimensional form by the reference
 *
 * This routine dimensionalizes a nondimensional term by multiplying the non-dimensional form
 * by a scalar, i.e. \f$f^\dagger=\frac{f}{f_ref}\f$, where \f$f^\dagger\f$ is the nondimensional
 * form and \f$f_{ref}\f$ is a reference scale with form particular to the interpretation of the
 * field. Note that for temperature in particular, there are still additional steps to
 * dimensionalize, because we do not define a nondimensional temperature simply as
 * \f$T^\dagger=\frac{T}{\Delta T_{ref}}\f$. But, this function just treats the characteristic scale
 * that would appear in the denominator.
 * @param[in] field physical interpretation of value to dimensionalize
 * @param[out] value value to dimensionalize
 */
void dimensionalize(const field::NekFieldEnum & field, double & value);

/**
 * Get the reference heat flux scale, \f$\rho C_pU\Delta T\f$
 * @return reference heat flux scale
 */
double referenceFlux();

/**
 * Get the reference heat source scale, \f$\rho C_pU\Delta T/L\f$
 * @return reference heat source scale
 */
double referenceSource();

/**
 * Get the reference length scale
 * @return reference length scale
 */
double referenceLength();

/**
 * Get the reference area scale
 * @return reference area scale
 */
double referenceArea();

/**
 * Get the reference volume scale
 * @return reference volume scale
 */
double referenceVolume();

// useful concept from Stack Overflow for templating MPI calls
template <typename T>
MPI_Datatype resolveType();

/**
 * Helper function for MPI_Allgatherv of results in NekRS
 * @param[in] base_counts once multiplied by 'multiplier', the number of counts on each rank
 * @param[in] input rank-local data
 * @param[out] output collected result
 * @param[in] multiplier constant multiplier to set on each count indicator
 */
template <typename T>
void
allgatherv(const std::vector<int> & base_counts, const T * input, T * output, const int multiplier)
{
  int * recvCounts = (int *)calloc(commSize(), sizeof(int));
  int * displacement = (int *)calloc(commSize(), sizeof(int));
  displacementAndCounts(base_counts, recvCounts, displacement, multiplier);

  MPI_Allgatherv(input,
                 recvCounts[commRank()],
                 resolveType<T>(),
                 output,
                 (const int *)recvCounts,
                 (const int *)displacement,
                 resolveType<T>(),
                 platform->comm.mpiComm);

  free(recvCounts);
  free(displacement);
}

} // end namespace nekrs
