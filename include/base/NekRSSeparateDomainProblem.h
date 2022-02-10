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
 * \brief Solve NekRS coupled to 1d thermal hydraulic code
 *
 * This object controls all of the execution of and data transfers to/from NekRS,
 * for coupling NekRS to a 1d thermal hydraulic code, such as SAM THM or RELAP7.
 *
 */
class NekRSSeparateDomainProblem : public NekRSProblemBase
{
public:
  NekRSSeparateDomainProblem(const InputParameters & params);

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
  ~NekRSSeparateDomainProblem();

  /**
   * \brief Perform some sanity checks on the problem setup
   *
   * This function performs checks like making sure that a transient executioner is
   * used to wrap nekRS, that no time shift has been requested to the start of nekRS,
   * that the correct NekTimeStepper is used, etc.
   */
  virtual void initialSetup() override;

  /// Send boundary velocity to nekRS
  void sendBoundaryVelocityToNek();

  /// Send boundary temperature to nekRS
  void sendBoundaryTemperatureToNek();

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



  virtual bool movingMesh() const override { return _moving_mesh; }

protected:
  std::unique_ptr<NumericVector<Number>> _serialized_solution;

  /// Whether the problem is a moving mesh problem i.e. with on-the-fly mesh deformation enabled
  const bool & _moving_mesh;

  const bool & _minimize_transfers_in;

  const bool & _minimize_transfers_out;

  /**
   * Send velocity from 1d system code to the nekRS mesh
   * @param[in] elem_id global element ID
   * @param[in] velocity_1dCode 1dCode's boundary velocity
   */
  void velocity(const int elem_id, const double velocity_1dCode);

  /**
   * Send temperature from 1d system code to the nekRS mesh
   * @param[in] elem_id global element ID
   * @param[in] temperature_1dCode 1dCode's boundary temperature
   */
  void temperature(const int elem_id, const double temperature_1dCode);

  /// Specify type of interfaces present for ExternalApp-NekRS coupling 
  const bool & _toNekRS;
  const bool & _toNekRS_temperature;
  const bool & _fromNekRS;
  const bool & _fromNekRS_temperature;

  /// Boundary ID for NekRS outlet
  const std::vector<int> * _outlet_boundary;

  /// Boundary ID for NekRS inlet
  const std::vector<int> * _inlet_boundary;

  /// Velocity boundary condition coming from external App to NekRS
  const PostprocessorValue * _toNekRS_velocity = nullptr;

  /// Temperature boundary condition coming from external App to NekRS
  const PostprocessorValue * _toNekRS_temp = nullptr;

  /// Postprocessor containing the signal of when a synchronization has occurred
  const PostprocessorValue * _transfer_in = nullptr;

  /// flag to indicate whether this is the first pass to serialize the solution
  static bool _first;
};
