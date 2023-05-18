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
 * for coupling NekRS to a 1d thermal hydraulic code, such as SAM, THM, or RELAP7.
 *
 */
class NekRSSeparateDomainProblem : public NekRSProblemBase
{
public:
  NekRSSeparateDomainProblem(const InputParameters & params);

  static InputParameters validParams();

  ~NekRSSeparateDomainProblem();

  virtual void initialSetup() override;

  /// Send boundary velocity to nekRS
  void sendBoundaryVelocityToNek(const nek_mesh::NekMeshEnum pp_mesh);

  /// Send boundary temperature to nekRS
  void sendBoundaryTemperatureToNek(const nek_mesh::NekMeshEnum pp_mesh);

  ///Send scalar to NekRS
  void sendBoundaryScalarToNek(const nek_mesh::NekMeshEnum pp_mesh,
                               const int scalarId,
                               const double scalarValue);

  virtual void syncSolutions(ExternalProblem::Direction direction) override;

  virtual void addExternalVariables() override;

protected:
  /**
   * Send velocity from 1d system code to the nekRS mesh
   * @param[in] pp_mesh which NekRS mesh to act on
   * @param[in] elem_id global element ID
   * @param[in] velocity boundary velocity
   */
  void velocity(const nek_mesh::NekMeshEnum pp_mesh, const int elem_id, const double velocity);

  /**
   * Send temperature from 1d system code to the nekRS mesh
   * @param[in] pp_mesh which NekRS mesh to act on
   * @param[in] elem_id global element ID
   * @param[in] temperature boundary temperature
   */
  void temperature(const nek_mesh::NekMeshEnum pp_mesh, const int elem_id, const double temperature);

  /**
   * Send scalar from 1d system code to the nekRS mesh
   * @param[in] pp_mesh which NekRS mesh to act on
   * @param[in] elem_id global element ID
   * @param[in] scalarId corresponding NekRS scalar ID
   * @param[in] scalarValue boundary scalar value
   */
  void scalar(const nek_mesh::NekMeshEnum pp_mesh, const int elem_id, const int scalarId, const double scalarValue);

  /// Type of coupling to apply to NekRS
  const MultiMooseEnum _coupling_type;

  /// Boundary ID for NekRS outlet
  const std::vector<int> & _outlet_boundary;

  /// Boundary ID for NekRS inlet
  const std::vector<int> & _inlet_boundary;

  /// Whether the NekRS inlet boundary is fed BCs from a coupled MOOSE app
  bool _inlet_coupling;

  /// Whether the NekRS outlet boundary feeds BCs to a coupled MOOSE app
  bool _outlet_coupling;

  /// Scalars coupled to NekRS
  const MultiMooseEnum _coupled_scalars;

  /// Whether the NekRS scalar01-03 are coupled to MOOSE app
  bool _scalar01_coupling;
  bool _scalar02_coupling;
  bool _scalar03_coupling;

  /// Velocity boundary condition coming from external App to NekRS
  const PostprocessorValue * _toNekRS_velocity = nullptr;

  /// Temperature boundary condition coming from external App to NekRS
  const PostprocessorValue * _toNekRS_temp = nullptr;

  /// Scalar01-03 boundary condition coming from external App to NekRS
  const PostprocessorValue * _toNekRS_scalar01 = nullptr;
  const PostprocessorValue * _toNekRS_scalar02 = nullptr;
  const PostprocessorValue * _toNekRS_scalar03 = nullptr;

  /// Postprocessor containing the signal of when a synchronization has occurred
  const PostprocessorValue * _transfer_in = nullptr;

  /// Which NekRS mesh to act on (always the fluid mesh)
  const nek_mesh::NekMeshEnum _pp_mesh = nek_mesh::fluid;
};
