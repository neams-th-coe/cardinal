#pragma once

#include "ExternalProblem.h"
#include "NekTimeStepper.h"
#include "NekRSMesh.h"
#include "Transient.h"

#include <memory>

class NekRSProblem;

template<>
InputParameters validParams<NekRSProblem>();

/**
 * \brief Solve nekRS wrapped as a MOOSE app.
 *
 * This object controls all of the execution of and data transfers to/from nekRS,
 * fully abstracted from the Nek5000 backend approach adopted in the similar, but
 * more limited, 'NekProblem' class.
 *
 * The nekRS temperature solution is interpolated onto the NekRSMesh by multiplying the
 * nekRS temperature by an interpolation matrix. In the opposite direction, the flux
 * from MOOSE is interpolated onto the nekRS mesh by a similar interpolation matrix.
 * This interpolation matrix expresses the
 * nekRS/MOOSE solutions in terms of interpolating Legendre polynomials, and is equal to
 * \f$V_{moose}V_{nek}^{-1}\f$, where \f$V_{moose}\f$ is the Vandermonde matrix of the
 * MOOSE mesh's node points and \f$V_{nek}\f$ is the Vandermonde matrix of the nekRS
 * mesh's 1-D quadrature points. If the interpolation matrix is unity, this means that
 * the libMesh node points exactly coincide with the nekRS quadrature point locations, and
 * hence no interpolation is actually needed.
 */
class NekRSProblem : public ExternalProblem
{
public:
  NekRSProblem(const InputParameters & params);

  /**
   * \brief Write nekRS's solution at the last output step
   *
   * If NekApp is not the master app, the number of time steps it takes is
   * controlled by the master app. Depending on the settings in the `.par` file,
   * it becomes possible that nekRS may not write an output file on the simulation's
   * actual last time step, because NekApp may not know when that last time step is.
   * Therefore, here we can force nekRS to write its output.
   **/
  ~NekRSProblem();

  /**
   * \brief Perform some sanity checks on the problem setup
   *
   * This function performs checks like making sure that a transient executioner is
   * used to wrap nekRS, that no time shift has been requested to the start of nekRS,
   * that the correct NekTimeStepper is used, etc.
   */
  virtual void initialSetup() override;

  /**
   * Fill an outgoing auxiliary variable field with nekRS solution data
   * \param[in] var_number auxiliary variable number
   * \param[in] value nekRS solution data to fill the variable with
   */
  virtual void fillAuxVariable(const unsigned int var_number, const double * value);

  /// Send boundary heat flux to nekRS
  void sendBoundaryHeatFluxToNek();

  /// Send volume mesh deformation flux to nekRS
  void sendVolumeDeformationToNek();
  
  /// Send volume heat source to nekRS
  void sendVolumeHeatSourceToNek();

  /// Get boundary temperature from nekRS
  void getBoundaryTemperatureFromNek();

  /// Get volume temperature from nekRS
  void getVolumeTemperatureFromNek();

  virtual void externalSolve() override;

  virtual void syncSolutions(ExternalProblem::Direction direction) override;

  virtual bool converged() override { return true; }

  virtual void addExternalVariables() override;

  /**
   * \brief Whether nekRS should write an output file for the current time step
   *
   * A nekRS output file (suffix .f000xx) is written if the time step is an integer
   * multiple of the output writing interval or if the time step is the last time step.
   * \return whether to write a nekRS output file
   **/
  virtual bool isOutputStep() const;

  /**
   * Whether data should be synchronized in to nekRS
   * \return whether inward data synchronization should occur
   */
  virtual bool synchronizeIn();

  /**
   * Whether data should be synchronized out of nekRS
   * \return whether outward data synchronization should occur
   */
  virtual bool synchronizeOut();

  /**
   * Determine the maximum interpolated temperature on the NekRSMesh for diagnostic info
   * \return maximum interpolated surface temperature
   */
  virtual double maxInterpolatedTemperature() const;

  /**
   * Determine the minimum interpolated temperature on the NekRSMesh for diagnostic info
   * \return minimum interpolated surface temperature
   */
  virtual double minInterpolatedTemperature() const;

protected:
  std::unique_ptr<NumericVector<Number>> _serialized_solution;

  /// Whether the problem is a moving mesh problem i.e. with on-the-fly mesh deformation enabled
  const bool & _moving_mesh;

  /**
   * \brief Whether to only send heat flux to nekRS on the multiapp synchronization steps
   *
   * nekRS is often subcycled relative to the application controlling it -
   * that is, nekRS may be run with a time step 10x smaller than a conduction MOOSE app.
   * The ExternalProblem interface in MOOSE, however, currently synchronizes (i.e. sends
   * data to/from nekRS) on _every_ nekRS time step. If 'interpolate_transfers = false'
   * in the master application, then the heat flux going into nekRS is fixed for each
   * of the subcycled time steps it takes, so these extra data transfers are
   * completely unnecssary. This flag indicates whether a somewhat hacky postprocessor
   * data transfer should be used for nekRS to figure out whether each time step is
   * the first immediately following a transfer from it's master application, and
   * then only send heat flux into nekRS's arrays if the data from MOOSE is new.
   *
   * NOTE: if 'interpolate_transfers = true' in the master application, then the data
   * coming into nekRS is _unique_ on each subcycled time step, so setting this to
   * true will in effect override `interpolate_transfers` to be false. For the best
   * performance, you should set `interpolate_transfers` to false so that you don't
   * even bother computing the interpolated data, since it's not used if this parameter
   * is set to true.
   */
  const bool & _minimize_transfers_in;

  /**
   * \brief Whether to only send temperature from nekRS on the multiapp synchronization steps
   *
   * nekRS is often subcycled relative to the application controlling it -
   * that is, nekRS may be run with a time step 10x smaller than a conduction MOOSE app.
   * The ExternalProblem interface in MOOSE, however, currently synchronizes (i.e. sends
   * data to/from nekRS) on _every_ nekRS time step. If 'interpolate_transfers = false'
   * in the master application, then the temperature coming from nekRS is fixed
   * (from the master's perspective) for each
   * of the subcycled time steps it takes, so these extra data transfers are
   * completely unnecssary. This flag indicates whether the coincidence of the
   * simulation time with the 'target_time' should be used to only transfer data out
   * of nekRS on the synchronization steps when that data will actually be used.
   *
   * NOTE: if 'interpolate_transfers = true' in the master application, then the data
   * sent from nekRS is _unique_ on each subcycled time step, so setting this to
   * true will in effect override `interpolate_transfers` to be false. For the best
   * performance, you should set `interpolate_transfers` to false so that you don't
   * even bother computing the interpolated data, since it's not used if this parameter
   * is set to true.
   */
  const bool & _minimize_transfers_out;

  /// Whether the nekRS solution is performed in nondimensional scales
  const bool & _nondimensional;

  //@{
  /**
   * \brief Reference scales for nekRS solution in non-dimensional form.
   *
   * If nekRS is solving in non-dimensional form, this means that the nekRS solution
   * is performed for:
   *
   * - nondimensional temperature \f$T^\dagger\f$, defined as
   *   \f$T^\dagger=\frac{T-T_{ref}}{\Delta T_{ref}}\f$.
   *   The 'T_ref' and 'dT_ref' variables here represent these scales.
   *
   * - nondimensional velocity \f$U^\dagger=\frac{u}{U_{ref}}\f$. The 'U_ref' variable here
   *   represents this velocity scale.
   *
   * - nondimensional pressure \f$P^dagger=\frac{P}{\rho_{0}U_{ref}^2}\f$. The 'rho_0'
   *   variable here represents this density parameter.
   *
   * In terms of heat flux boundary conditions, the entire energy conservation equation
   * in nekRS is multiplied by \f$\frac{L_{ref}}{\rho_{0}C_{p,0}U_{ref}\Delta T_{ref}}\f$
   * in order to clear the coefficient on the convective. Therefore, the heat source in
   * nekRS is expressed in nondimensional form as \f$q^\dagger=\frac{q}{\rho_{0}C_{p,0}U_{ref}\Delta T_{ref}}\f$.
   * Here, 'Cp_0' is the specific heat capacity scale.
   *
   * Unfortunately, there is no systematic way to get these reference
   * scales from nekRS, so we need to inform MOOSE of any possible scaling so that we
   * can appropriately scale the nekRS temperature to the dimensional form that is usually
   * expected in MOOSE. Therefore, these scales just need to match what is used in the nekRS
   * input files used to specify boundary conditions. Conversion between dimensional MOOSE
   * heat flux to dimensionless nekRS heat flux is performed automatically, and does not require
   * any special treatment in the nekRS scalarNeumannBC kernel.
   */
  const Real & _U_ref;

  const Real & _T_ref;

  const Real & _dT_ref;

  const Real & _L_ref;

  const Real & _rho_0;

  const Real & _Cp_0;
  //@}

  /// Start time of the simulation based on nekRS's .par file
  double _start_time;

  /// The time stepper used for selection of time step size
  NekTimeStepper * _timestepper = nullptr;

  /**
   * \brief Total surface-integrated flux coming from the coupled MOOSE app.
   *
   * The mesh used for the MOOSE app may be very different from the mesh used by nekRS.
   * Elements may be much finer/coarser, and one element on the MOOSE app may not be a
   * clear subset/superset of the elements on the nekRS mesh. Therefore, to ensure
   * conservation of energy, we send the total flux integral to nekRS for internal
   * normalization of the heat flux applied on the nekRS mesh.
   */
  const PostprocessorValue * _flux_integral = nullptr;

  /**
   * \brief Total volume-integrated heat source coming from the coupled MOOSE app.
   *
   * The mesh used for the MOOSE app may be very different from the mesh used by nekRS.
   * Elements may be much finer/coarser, and one element on the MOOSE app may not be a
   * clear subset/superset of the elements on the nekRS mesh. Therefore, to ensure
   * conservation of energy, we send the total source integral to nekRS for internal
   * normalization of the heat source applied on the nekRS mesh.
   */
  const PostprocessorValue * _source_integral = nullptr;

  /// Postprocessor to limit the minimum temperature
  const PostprocessorValue * _min_T = nullptr;

  /// Postprocessor to limit the maximum temperature
  const PostprocessorValue * _max_T = nullptr;

  /// nekRS temperature interpolated onto the data transfer mesh
  double * _T = nullptr;

  /// MOOSE flux interpolated onto the (boundary) data transfer mesh
  double * _flux_face = nullptr;

  /// MOOSE flux interpolated onto the (volume) data transfer mesh
  double * _flux_elem = nullptr;

  /// MOOSE heat source interpolated onto the data transfer mesh
  double * _source_elem = nullptr;

  /// displacement in x for all nodes from MOOSE, for moving mesh problems
  double * _displacement_x = nullptr;

  /// displacement in y for all nodes from MOOSE, for moving mesh problems
  double * _displacement_y = nullptr;

  /// displacement in z for all nodes from MOOSE, for moving mesh problems
  double * _displacement_z = nullptr;

  /// temperature transfer variable written to be nekRS
  unsigned int _temp_var;

  /// flux transfer variable read from by nekRS
  unsigned int _avg_flux_var;

  /// x-displacment transfer variable read from for moving mesh problems
  unsigned int _disp_x_var;

  /// y-displacment transfer variable read from for moving mesh problems
  unsigned int _disp_y_var;

  /// z-displacment transfer variable read from for moving mesh problems
  unsigned int _disp_z_var;

 /// volumetric heat source variable read from by nekRS
  unsigned int _heat_source_var;

  /// Descriptive string for data transfer going in to nekRS
  std::string _incoming;

  /// Descriptive string for data transfer coming from nekRS
  std::string _outgoing;

  /// Number of points for interpolated fields (temperature, density) on the MOOSE mesh
  int _n_points;

  /// Postprocessor containing the signal of when a synchronization has occurred
  const PostprocessorValue * _transfer_in = nullptr;

  /// Number of surface elements in the data transfer mesh, across all processes
  int _n_surface_elems;

  /// Number of vertices per surface element of the transfer mesh
  int _n_vertices_per_surface;

  /// Number of volume elements in the data transfer mesh, across all processes
  int _n_volume_elems;

  /// Number of vertices per volume element of the transfer mesh
  int _n_vertices_per_volume;

  /// Number of elements in the data transfer mesh, which depends on whether boundary/volume coupling
  int _n_elems;

  /// Number of vertices per element in the data transfer mesh, which depends on whether boundary/volume coupling
  int _n_vertices_per_elem;

  /// Boundary IDs through which to couple nekRS and MOOSE
  const std::vector<int> * _boundary = nullptr;

  /// Whether the mesh contains volume-based coupling
  bool _volume;

  /// Underlying mesh object on which nekRS exchanges fields with MOOSE
  const NekRSMesh* _nek_mesh;

  /// Underlying executioner
  Transient * _transient_executioner = nullptr;

  /// flag to indicate whether this is the first pass to serialize the solution
  static bool _first;

  /**
   * Whether an interpolation needs to be performed on the nekRS temperature solution, or
   * if we can just grab the solution at specified points
   */
  bool _needs_interpolation;
};
