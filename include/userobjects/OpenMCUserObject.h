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

// forward declarations
class OpenMCProblemBase;

/**
 * User object to modify an OpenMC object
 */
class OpenMCUserObject : public GeneralUserObject
{
public:
  static InputParameters validParams();

  OpenMCUserObject(const InputParameters & parameters);

  void execute() override;

  std::string long_name() const { return "OpenMCUserObject \"" + this->name() + "\""; }

  const OpenMCProblemBase * openmc_problem() const;

protected:
  bool _first_execution;
};
