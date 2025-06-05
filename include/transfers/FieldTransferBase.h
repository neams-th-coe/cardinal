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

class NekRSProblemBase;

/**
 * Base class for facilitating a data transfer between MOOSE and the NekRS
 * code internals.
 */
class FieldTransferBase : public MooseObject
{
public:
  static InputParameters validParams();

  FieldTransferBase(const InputParameters & parameters);

  /**
   * Add a MOOSE variable to facilitate coupling
   * @param[in] name variable name
   */
  void addExternalVariable(const std::string name);

  /**
   * Variable name added by this object
   * @return variable name
   */
  std::string variableName() const { return _variable; }

protected:
  /// The NekRSProblem using the field transfer interface
  NekRSProblemBase & _nek_problem;

  /// Direction of the transfer
  const MooseEnum & _direction;

  /// Variable name to create in MOOSE to facilitate data passing
  std::string _variable;
};
