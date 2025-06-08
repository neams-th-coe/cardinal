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

/// Couples NekRS and MOOSE through mesh displacements
class NekMeshDeformation : public FieldTransferBase
{
public:
  static InputParameters validParams();

  NekMeshDeformation(const InputParameters & parameters);

  ~NekMeshDeformation();

  virtual void sendDataToNek() override;

protected:
  /// Send boundary deformation to nekRS
  void sendBoundaryDeformationToNek();

  /// Send volume mesh deformation flux to nekRS
  void sendVolumeDeformationToNek();

  /**
   * Calculate mesh velocity for NekRS's blending solver using current and previous displacement
   * values and write it to nrs->usrwrk, from where it can be accessed in nekRS's .oudf file.
   * @param[in] e Boundary element that the displacement values belong to
   * @param[in] field NekWriteEnum mesh_velocity_x/y/z field
   */
  void calculateMeshVelocity(int e, const field::NekWriteEnum & field);

  /// displacement in x for all nodes from MOOSE, for moving mesh problems
  double * _displacement_x = nullptr;

  /// displacement in y for all nodes from MOOSE, for moving mesh problems
  double * _displacement_y = nullptr;

  /// displacement in z for all nodes from MOOSE, for moving mesh problems
  double * _displacement_z = nullptr;

  /// mesh velocity for a given element, used internally for calculating mesh velocity over one element
  double * _mesh_velocity_elem = nullptr;
};
