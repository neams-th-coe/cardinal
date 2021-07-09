#pragma once

#include "CardinalEnums.h"
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
 * Get the mesh for the temperature scalar
 * @return temperature mesh
 */
mesh_t * temperatureMesh();

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
 * Whether nekRS contains an OCCA kernel to apply a source to the passive scalar equations
 * @return whether nekRS has an OCCA kernel for apply a passive scalar source
 */
bool hasHeatSourceKernel();

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
void copyScratchToDevice();

/// Copy volume deformation of mesh from host to device for moving-mesh problems
void copyDeformationToDevice();

/**
 * Determine the receiving counts and displacements for all gather routines
 * @param[in] base_counts unit-wise receiving counts for each process
 * @param[out] counts receiving counts from each process
 * @param[out] displacement displacement for each process's counts
 * @param[in] multiplier optional multiplier on the face-based data
 */
void displacementAndCounts(const int * base_counts, int * counts, int * displacement, const int multiplier);

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
void interpolateSurfaceFaceHex3D(double * scratch, const double* I, double* x, int N, double* Ix, int M);

/**
 * Initialize interpolation matrices for transfers in/out of nekRS
 * @param[in] n_moose_pts number of MOOSE quadrature points in 1-D
 */
void initializeInterpolationMatrices(const int n_moost_pts);

/**
 * Interpolate the nekRS temperature onto the boundary data transfer mesh
 * @param[in] order enumeration of the surface mesh order (0 = first, 1 = second, etc.)
 * @param[in] needs_interpolation whether an interpolation matrix needs to be used to figure out the interpolation
 * @param[out] T interpolated temperature
 */
void boundaryTemperature(const int order, const bool needs_interpolation, double* T);

/**
 * Interpolate the nekRS temperature onto the volume data transfer mesh
 * @param[in] order enumeration of the mesh order (0 = first, 1 = second, etc.)
 * @param[in] needs_interpolation whether an interpolation matrix needs to be used to figure out the interpolation
 * @param[out] T interpolated temperature
 */
void volumeTemperature(const int order, const bool needs_interpolation, double* T);

/**
 * Interpolate the MOOSE flux onto the nekRS mesh
 * @param[in] elem_id global element ID
 * @param[in] order enumeration of the surface mesh order (0 = first, 1 = second, etc.)
 * @param[in] flux_face flux at the libMesh nodes
 */
 void flux(const int elem_id, const int order, double * flux_face);

/**
 * Interpolate a volume-based MOOSE flux into the nekRS mesh
 * @param[in] elem_id global element ID
 * @param[in] order enumeration of the surface mesh order (0 = first, 1 = second, etc.)
 * @param[in] flux_elem flux at the libMesh nodes
 */
void flux_volume(const int elem_id, const int order, double * flux_elem);

/**
 * Interpolate the MOOSE volume heat source onto the nekRS mesh
 * @param[in] elem_id global element ID
 * @param[in] order enumeration of the volume mesh order (0 = first, 1 = second, etc.)
 * @param[in] source_elem heat source at the libMesh nodes
 */
void heat_source(const int elem_id, const int order, double * source_elem);

/**
 * Save the initial mesh in nekRS for moving mesh problems
 */
void save_initial_mesh();

//@{
/**
 * Initial nekRS mesh coordinates saved to apply time-dependent volume deformation to the initial
 * nekRS mesh in order to make the deformation congruent to MOOSE-applied deformation
*/
static double * initial_mesh_x = nullptr;                                                                                                static double * initial_mesh_y = nullptr;
static double * initial_mesh_z = nullptr;
//@}

/**
 * Used to check if the initial nekRS mesh has already been saved, and to prevent accidental
 * calls to save_initial_mesh() that may overwrite the previously saved initial mesh
 */
static bool is_saved_initial_mesh = false; // hello

//@{
/**
 * Interpolate the MOOSE volume mesh displacement onto the nekRS mesh
 * @param[in] elem_id global element ID
 * @param[in] order enumeration of the volume mesh order (0 = first, 1 = second, etc.)
 * @param[in] disp_vol displacement at the libMesh nodes
 */
void map_volume_x_deformation(const int elem_id, const int order, double * disp_vol);
void map_volume_y_deformation(const int elem_id, const int order, double * disp_vol);
void map_volume_z_deformation(const int elem_id, const int order, double * disp_vol);
//@}

/**
 * Integrate the interpolated flux over the boundaries of the data transfer mesh
 * @return boundary integrated flux
 */
double fluxIntegral();

/**
 * Integrate the interpolated heat source over the volume of the data transfer mesh
 * @return volume integrated heat source
 */
double sourceIntegral();

/**
 * Normalize the flux sent to nekRS to conserve the total flux
 * @param[in] moose_integral total integrated flux from MOOSE to conserve
 * @param[in] nek_integral total integrated flux in nekRS to adjust
 * @param[out] normalized_nek_integral final normalized nek flux integral
 * @return whether normalization was successful, i.e. normalized_nek_integral equals moose_integral
 */
bool normalizeFlux(const double moose_integral, double nek_integral, double & normalized_nek_integral);

/**
 * Normalize the heat source sent to nekRS to conserve the total heat source
 * @param[in] moose_integral total integrated heat source from MOOSE to conserve
 * @param[in] nek_integral total integrated heat source in nekRS to adjust
 * @param[out] normalized_nek_integral final normalized nek source integral
 * @return whether normalization was successful, i.e. normalized_nek_integral equals moose_integral
 */
bool normalizeHeatSource(const double moose_integral, const double nek_integral, double & normalized_nek_integral);

/**
 * Compute the area of a set of boundary IDs
 * @param[in] boundary_id nekRS boundary IDs for which to perform the integral
 * @return area integral
 */
double area(const std::vector<int> & boundary_id);

/**
 * Compute the area integral of a given integrand over a set of boundary IDs
 * @param[in] boundary_id nekRS boundary IDs for which to perform the integral
 * @param[in] integrand field to integrate
 * @return area integral of a field
 */
double sideIntegral(const std::vector<int> & boundary_id, const field::NekFieldEnum & integrand);

/**
 * Compute the volume over the entire scalar mesh
 * @return volume integral
 */
double volume();

/**
 * Compute the volume integral of a given integrand over the entire scalar mesh
 * @param[in] integrand field to integrate
 * @return volume integral of a field
 */
double volumeIntegral(const field::NekFieldEnum & integrand);

/**
 * Compute the mass flowrate over a set of boundary IDs
 * @param[in] boundary_id nekRS boundary IDs for which to compute the mass flowrate
 * @return mass flowrate
 */
double massFlowrate(const std::vector<int> & boundary_id);

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
 * Limit the temperature in nekRS to within the range of [min_T, max_T]
 * @param[in] min_T minimum temperature allowable in nekRS
 * @param[in] max_T maximum temperature allowable in nekRS
 */
void limitTemperature(const double * min_T, const double * max_T);

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
struct interpolationMatrix
{
  /**
   * \brief Interpolation matrix to interpolate _from_ a MOOSE mesh to the nekRS mesh
   *
   * This interpolation matrix is used to interpolate boundary heat flux (for boundary
   * coupling) or volume power density (for volume coupling) from a MOOSE mesh to nekRS's mesh.
   */
  double * incoming;

  /**
   * \brief Interpolation matrix to interpolate _from_ a nekRS mesh to a MOOSE mesh
   *
   * This interpolation matrix is used to interpolate boundary temperature (for boundary
   * coupling) or volume temperatures and densities (for volume coupling) from nekRS's mesh
   * to a MOOSE mesh.
   */
  double * outgoing;
};

/// Store the geometry and parallel information related to the volume mesh coupling
struct volumeCoupling
{
  // process-local element IDS (for all elements)
  int * element;

  // process owning each element (for all elements)
  int * process;

  // number of elements owned by each process
  int * counts;

  // offset into the boundary_coupling array where this element's face data begins
  // (this value is not initialized if that element doesnt have any faces on a boundary)
  int * boundary_offset;

  // number of faces on a boundary of interest for each element
  int * n_faces_on_boundary;

  // number of coupling elements owned by this process
  int n_elems;

  // total number of coupling elements
  int total_n_elems;

  /**
   * nekRS process owning the global element in the data transfer mesh
   * @param[in] elem_id element ID
   * @return nekRS process ID
   */
  int processor_id(const int elem_id) { return process[elem_id]; }
};

/// Store the geometry and parallel information related to the surface mesh coupling
struct boundaryCoupling
{
  // process-local element IDS on the boundary of interest (for all ranks)
  int * element;

  // element-local face IDs on the boundary of interest (for all ranks)
  int * face;

  // problem-global boundary ID for each element (for all ranks)
  int * boundary_id;

  // process owning each face (for all faces)
  int * process;

  // number of faces owned by each process
  int * counts;

  // number of coupling elements owned by this process
  int n_faces;

  // total number of coupling elements
  int total_n_faces;

  // offset into the element, face, and process arrays where this rank's data begins
  int offset;

  /**
   * nekRS process owning the global element in the data transfer mesh
   * @param[in] elem_id element ID
   * @return nekRS process ID
   */
  int processor_id(const int elem_id) { return process[elem_id]; }
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
 * Processor id (rank) owning the given volume element
 * @return processor id
 */
int VolumeElemProcessorID(const int elem_id);

/**
 * Processor id (rank) owning the given boundary element
 * @return processor id
 */
int BoundaryElemProcessorID(const int elem_id);

/**
 * Store the rank-local element, element-local face, and rank ownership for boundary coupling
 * @param[in] boundary_id boundaries through which nekRS will be coupled
 * @param[out] N total number of surface elements
 */
void storeBoundaryCoupling(const std::vector<int> & boundary_id, int& N);

/**
 * \brief Get the vertices defining the surface mesh interpolation from the stored coupling information
 * @param[in] order enumeration of the surface mesh order (0 = first, 1 = second, etc.)
 * @param[out] x Array of \f$x\f$-coordinates for face vertices
 * @param[out] y Array of \f$y\f$-coordinates for face vertices
 * @param[out] z Array of \f$z\f$-coordinates for face vertices
 */
void faceVertices(const int order, double* x, double* y, double* z);

/**
 * Store the rank-local element and rank ownership for volume coupling
 * @param[out] N total number of volume elements
 */
void storeVolumeCoupling(int& N);

/**
 * \brief Get the vertices defining the volume mesh interpolation and store mesh coupling information
 * @param[in] order enumeration of the volume mesh order (0 = first, 1 = second, etc.)
 * @param[out] x Array of \f$x\f$-coordinates for element vertices
 * @param[out] y Array of \f$y\f$-coordinates for element vertices
 * @param[out] z Array of \f$z\f$-coordinates for element vertices
 */
void volumeVertices(const int order, double* x, double* y, double* z);

/**
 * Get the number of faces of this global element that are on a coupling boundary
 * @param[in] elem_id global element ID
 * @return number of faces on a couling boundary
 */
int facesOnBoundary(const int elem_id);

/**
 * Get the element-local face ID and sideset ID for a given global element
 * @param[in] elem_id global element ID
 * @param[in] face_id global face ID
 * @param[out] face local face ID
 * @param[out] side boundary ID for the face
 */
void faceSideset(const int elem_id, const int face_id, int& face, int& side);

/// Free dynamically allocated memory related to the surface mesh interpolation
void freeMesh();

} // end namespace mesh

namespace solution
{

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
 * Get pointer to various solution functions based on enumeration
 * @param[in] field field to return a pointer to
 * @return function pointer to method that returns said field as a function of GLL index
 */
double (*solutionPointer(const field::NekFieldEnum & field))(int);

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
 * Initialize the characteristic scales for a nondimesional solution
 * @param[in] U_ref reference velocity
 * @param[in] T_ref reference temperature
 * @param[in] dT_ref reference temperature range
 * @param[in] L_ref reference length scale
 * @param[in] rho_ref reference density
 * @param[in] Cp_ref reference heat capacity
 */
void initializeDimensionalScales(const double U_ref, const double T_ref, const double dT_ref,
  const double L_ref, const double rho_ref, const double Cp_ref);

/**
 * \brief Dimensionalize a field by multiplying the nondimensional form by the reference
 *
 * This routine dimensionalizes a nondimensional term by multiplying the non-dimensional form
 * by a scalar, i.e. \f$f^\dagger=\frac{f}{f_ref}\f$, where \f$f^\dagger\f$ is the nondimensional
 * form and \f$f_{ref}\f$ is a reference scale with form particular to the interpretation of the
 * field. Note that for temperature in particular, there are still additional steps to
 * dimensionalize, because we do not define a nondimensional temperature simply as \f$T^\dagger=\frac{T}{\DeltaT_{ref}}\f$.
 * But, this function just treats the characteristic scale that would appear in the denominator.
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

} // end namespace solution

} // end namespace nekrs
