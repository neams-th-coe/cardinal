/********************************************************************/
/*                  SOFTWARE COPYRIGHT NOTIFICATION                 */
/*                             Cardinal                             */
/*                                                                  */
/*                  (c) 2024 UChicago Argonne, LLC                  */
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

#include "GeneralUserObject.h"

class OpenMCProblemBase;

/**
 * User object that acts on aspects of the OpenMC simulation
 */
class OpenMCUserObject : public GeneralUserObject
{
public:
  static InputParameters validParams();

  OpenMCUserObject(const InputParameters & parameters);

  /// We don't want this user object to execute in MOOSE's control
  virtual void initialize() {}
  virtual void finalize() {}
  virtual void execute() {}

  std::string longName() const { return "OpenMCUserObject \"" + this->name() + "\""; }

  const OpenMCProblemBase * openmcProblem() const;
};
