#pragma once

#include "CardinalEnums.h"
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
 * Whether the scratch space has already been allocated by the user
 * @return whether scratch space is already allocated
 */
bool scratchAvailable();

/// Initialize scratch space for flux transfer
void initializeScratch();

/// Free the scratch space for the flux transfer
void freeScratch();

/// Copy the flux from host to device
void copyFluxToDevice();

/**
 * nekRS process owning the global element in the data transfer mesh
 * @param[in] elem_id element ID
 * @return nekRS process ID
 */
int processor_id(const int elem_id);

/**
 * Determine the receiving counts and displacements for all gather routines
 * @param[out] counts receiving counts from each process
 * @param[out] displacement displacement for each process's counts
 * @param[in] multiplier optional multiplier on the face-based data
 */
void displacementAndCounts(int * counts, int * displacement, const int multiplier);

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
void interpolateSurfaceFaceHex3D(double * scratch, double* I, double* x, int N, double* Ix, int M);

/**
 * Interpolate the nekRS temperature onto the data transfer mesh
 * @param[in] I interpolation matrix
 * @param[in] order enumeration of the surface mesh order (0 = first, 1 = second, etc.)
 * @param[in] needs_interpolation whether an interpolation matrix needs to be used to figure out the interpolation
 * @param[out] T interpolated temperature
 */
void temperature(const double * I, const int order, const bool needs_interpolation, double* T);

/**
 * Interpolate the MOOSE flux onto the nekRS mesh
 * @param[in] I interpolation matrix
 * @param[in] elem_id global element ID
 * @param[in] order enumeration of the surface mesh order (0 = first, 1 = second, etc.)
 * @param[in] flux_face flux at the libMesh nodes
 */
 void flux(const double * I, const int elem_id, const int order, double * flux_face);

/**
 * Integrate the interpolated flux over the boundaries of the data transfer mesh
 * @return boundary integrated flux
 */
double fluxIntegral();

/**
 * Normalize the flux sent to nekRS to conserve the total flux
 * @param[in] moose_integral total integrated flux from MOOSE to conserve
 * @param[in] nek_integral total integrated flux in nekRS to adjust
 */
void normalizeFlux(const double moose_integral, const double nek_integral);

/**
 * Compute the area integral of a given integrand over a set of boundary IDs
 * @param[in] boundary_id nekRS boundary IDs for which to perform the integral
 * @param[in] integrand field to integrate
 * @return area integral of a field
 */
double sideIntegral(const std::vector<int> & boundary_id, const field::NekFieldEnum & integrand);

/**
 * Compute the mass flux weighted integral of a given integrand over a set of boundary IDs
 * @param[in] boundary_id nekRS boundary IDs for which to perform the integral
 * @param[in] integrand field to integrate and weight by mass flux
 * @return mass flux weighted area average of a field
 */
double sideMassFluxWeightedIntegral(const std::vector<int> & boundary_id, const field::NekFieldEnum & integrand);

/**
 * Compute the heat flux over a set of boundary IDs
 * @param[in] boundary_id nekRS boundary IDs for which to perform the integral
 * @return heat flux area integral
 */
double heatFluxIntegral(const std::vector<int> & boundary_id);

/**
 * Compute the gradient of a volume field
 * @param[in] offset in the gradient field for each component (grad_x, grad_y, or grad_z)
 * @param[in] f field to compute the gradient of
 * @param[out] grad_f gradient of field
 */
void gradient(const int offset, const double * f, double * grad_f);

/**
 * Find the minimum of a given field over the entire nekRS domain
 * @param[in] field field to find the minimum value of
 * @return minimum value of field in volume
 */
double volumeMinValue(const field::NekFieldEnum & field);

/**
 * Find the maximum of a given field over the entire nekRS domain
 * @param[in] field field to find the minimum value of
 * @return maximum value of field in volume
 */
double volumeMaxValue(const field::NekFieldEnum & field);

/**
 * Find the minimum of a given field over a set of boundary IDs
 * @param[in] boundary_id nekRS boundary IDs for which to find the extreme value
 * @param[in] field field to find the minimum value of
 * @return minimum value of field on boundary
 */
double sideMinValue(const std::vector<int> & boundary_id, const field::NekFieldEnum & field);

/**
 * Find the maximum of a given field over a set of boundary IDs
 * @param[in] boundary_id nekRS boundary IDs for which to find the extreme value
 * @param[in] field field to find the maximum value of
 * @param maximum value of field on boundary
 */
double sideMaxValue(const std::vector<int> & boundary_id, const field::NekFieldEnum & field);

namespace mesh
{

/// Store the geometry and parallel information related to the surface mesh coupling
struct boundaryCoupling
{
  // process-local element IDS on the boundary of interest (for all ranks)
  int * element;

  // element-local face IDs on the boundary of interest (for all ranks)
  int * face;

  // process owning each face (for all faces)
  int * process;

  // number of faces owned by each process
  int * counts;

  // number of coupling elements owned by this process
  int n_faces;

  // total number of coupling elements
  int total_n_faces;
};

/**
 * Whether the specific boundary is a flux boundary
 * @param[in] boundary boundary ID
 * @return whether boundary is a flux boundary
 */
bool isHeatFluxBoundary(const int boundary);

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
bool validBoundaryIDs(const std::vector<int> & boundary_id, int & first_invalid_id, int & n_boundaries);

/**
 * \brief Get the vertices defining the surface mesh interpolation and store mesh coupling information
 * @param[in] boundary_id nekRS boundary IDs for which to find the face vertices
 * @param[in] order enumeration of the surface mesh order (0 = first, 1 = second, etc.)
 * @param[out] x Array of \f$x\f$-coordinates for face vertices
 * @param[out] y Array of \f$y\f$-coordinates for face vertices
 * @param[out] z Array of \f$z\f$-coordinates for face vertices
 * @param[out] N number of face vertices in surface mesh
 */
void faceVertices(const std::vector<int> & boundary_id, const int order, float* x, float* y, float* z, int& N);

/// Free dynamically allocated memory related to the surface mesh interpolation
void freeMesh();

} // end namespace mesh

namespace solution
{

/**
 * \brief Get the temperature solution at given GLL index
 *
 * Because nekRS stores all the passive scalars together in one flat array, this routine
 * simply indices into the entire passive scalar solution. In order to get temperature, you should
 * only index up to nrs->cds->fieldOffset.
 * @param[in] id GLL index
 * @return temperature value at index
 */
const double temperature(const int id);

/**
 * Return unity, for cases where the integrand or operator we are generalizing acts on 1
 * @param[in] id GLL index
 * @return unity
 */
const double unity(const int id);

} // end namespace solution

} // end namespace nekrs
