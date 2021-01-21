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

  /// Whether diagnostic information should be printed to the console
  const bool & _verbose;

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

  /// The time stepper used for selection of time step size
  NekTimeStepper * _timestepper;

  /**
   * \brief Total integrated flux coming from the coupled MOOSE app.
   *
   * The mesh used for the MOOSE app may be very different from the mesh used by nekRS.
   * Elements may be much finer/coarser, and one element on the MOOSE app may not be a
   * clear subset/superset of the elements on the nekRS mesh. Therefore, to ensure
   * conservation of energy, we send the total flux integral to nekRS for internal
   * normalization of the heat flux applied on the nekRS mesh.
   */
  const PostprocessorValue * _flux_integral;

  /// nekRS temperature interpolated onto the data transfer mesh
  double * _T;

  /// MOOSE flux interpolated onto the data transfer mesh
  double * _flux_face;

  /// temperature transfer variable written to be nekRS
  unsigned int _temp_var;

  /// flux transfer variable read from by nekRS
  unsigned int _avg_flux_var;

  /// Start time of the simulation based on nekRS's .par file
  double _start_time;

  /// Name of postprocessor containing signal of when a synchronization has occurred
  const PostprocessorName * _transfer_in_name;

  /// Postprocessor containing the signal of when a synchronization has occurred
  const PostprocessorValue * _transfer_in;

  /// Number of surface elements in the data transfer mesh, across all processes
  int _n_surface_elems;

  /// Number of vertices per face of the transfer mesh
  int _n_vertices_per_face;

  /// Boundary IDs through which to couple nekRS and MOOSE
  std::vector<int> _boundary;

  /// Underlying mesh object on which nekRS exchanges fields with MOOSE
  const NekRSMesh* _nek_mesh;

  /// Underlying executioner
  Transient * _transient_executioner;

  /**
   * \brief Interpolation matrix between nekRS temperature solution and representation on the MOOSE mesh
   *
   * The nekRS temperature solution is interpolated onto the NekRSMesh by multiplying the
   * nekRS temperature by an interpolation matrix. This interpolation matrix expresse the
   * nekRS solution in terms of interpolating Legendre polynomials, and is equal to
   * \f$V_{moose}V_{nek}^{-1}\f$, where \f$V_{moose}\f$ is the Vandermonde matrix of the
   * MOOSE mesh's node points and \f$V_{nek}\f$ is the Vandermonde matrix of the nekRS
   * mesh's 1-D quadrature points. If the interpolation matrix is unity, this means that
   * the libMesh node points exactly coincide with the nekRS quadrature point locations, and
   * hence no interpolation is actually needed.
   */
   double * _temp_interpolation_matrix;

  /**
   * \brief Interpolation matrix between MOOSE flux and representation on the nekRS mesh
   *
   * The MOOSE flux solution is interpolated onto the nekRS mesh by multiplying the
   * MOOSE flux by an interpolation matrix. This interpolation matrix expresse the
   * MOOSE solution in terms of interpolating Legendre polynomials, and is equal to
   * \f$V_{nek}V_{moose}^{-1}\f$, where \f$V_{moose}\f$ is the Vandermonde matrix of the
   * MOOSE mesh's node points and \f$V_{nek}\f$ is the Vandermonde matrix of the nekRS
   * mesh's 1-D quadrature points. If the interpolation matrix is unity, this means that
   * the libMesh node points exactly coincide with the nekRS quadrature point locations, and
   * hence no interpolation is actually needed.
   */
   double * _flux_interpolation_matrix;

  /**
   * Whether an interpolation needs to be performed on the nekRS temperature solution, or
   * if we can just grab the solution at specified points
   */
  bool _needs_interpolation;
};
