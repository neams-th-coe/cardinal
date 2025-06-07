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

#include "MooseObject.h"
#include "PostprocessorInterface.h"

class NekRSProblem;
class NekRSMesh;

/**
 * Base class for facilitating a data transfer between MOOSE and the NekRS
 * code internals.
 */
class NekTransferBase : public MooseObject, public PostprocessorInterface
{
public:
  static InputParameters validParams();

  NekTransferBase(const InputParameters & parameters);

  /// Send data to Nek
  virtual void sendDataToNek() {}

  /// Read data from Nek
  virtual void readDataFromNek() {}

  /// Get the direction of the transfer
  const MooseEnum & direction() const { return _direction; }

protected:
  /**
   * Add a postprocessor to facilitate coupling
   * @param[in] name postprocessor name
   * @param[in] initial initial value to use for the postprocessor
   */
  void addExternalPostprocessor(const std::string name, const Real initial);

  /**
   * Check if a specified slot in the usrwrk array has been allocated
   * @param[in] u slice in usrwrk array
   */
  void checkAllocatedUsrwrkSlot(const unsigned int & u) const;

  /// The NekRSProblem using the field transfer interface
  NekRSProblem & _nek_problem;

  /// The underlying NekRSMesh mirror
  NekRSMesh * _nek_mesh;

  /// Direction of the transfer
  const MooseEnum & _direction;
};
