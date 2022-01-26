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

#include "NekRSProblemBase.h"
#include "NekTimeStepper.h"
#include "NekRSMesh.h"
#include "Transient.h"

#include <memory>

/**
 * \brief Solve nekRS wrapped as a MOOSE app.
 *
 * This object controls all of the execution of and data transfers to/from nekRS,
 * fully abstracted from the Nek5000 backend.
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

  static InputParameters validParams();

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

  /// flag to indicate whether this is the first pass to serialize the solution
  static bool _first;
};
