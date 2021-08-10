#pragma once

#include "NekRSProblemBase.h"
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
class NekRSProblem : public NekRSProblemBase
{
public:
  NekRSProblem(const InputParameters & params);

  /**
   * \brief Write nekRS's solution at the last output step
   *
   * If Nek is not the master app, the number of time steps it takes is
   * controlled by the master app. Depending on the settings in the `.par` file,
   * it becomes possible that nekRS may not write an output file on the simulation's
   * actual last time step, because Nek may not know when that last time step is.
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

  /**
   * Adjust the NekRS solution by introducing max/min temperature clipping
   * to help with underresolved flow
   */
  virtual void adjustNekSolution() override;

  virtual void syncSolutions(ExternalProblem::Direction direction) override;

  virtual void addExternalVariables() override;

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

  virtual bool movingMesh() const override { return _moving_mesh; }

protected:
  virtual void addTemperatureVariable() override { return; }

  std::unique_ptr<NumericVector<Number>> _serialized_solution;

  /// Whether the problem is a moving mesh problem i.e. with on-the-fly mesh deformation enabled
  const bool & _moving_mesh;

  /**
   * \brief Whether to only send heat flux to nekRS on the multiapp synchronization steps
   *
   * nekRS is often subcycled relative to the application controlling it -
   * that is, nekRS may be run with a time step 10x smaller than a conduction MOOSE app.
   * The NekRSProblemBase interface in MOOSE, however, currently synchronizes (i.e. sends
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
   * The NekRSProblemBase interface in MOOSE, however, currently synchronizes (i.e. sends
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

  /// Whether a heat source will be applied to NekRS from MOOSE
  const bool & _has_heat_source;

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

  /// Postprocessor containing the signal of when a synchronization has occurred
  const PostprocessorValue * _transfer_in = nullptr;

  /// flag to indicate whether this is the first pass to serialize the solution
  static bool _first;
};
