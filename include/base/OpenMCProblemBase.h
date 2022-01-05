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

#include "ExternalProblem.h"

/**
 * Base class for all MOOSE wrappings of OpenMC
 */
class OpenMCProblemBase : public ExternalProblem
{
public:
  OpenMCProblemBase(const InputParameters & params);

  static InputParameters validParams();

  ~OpenMCProblemBase();

protected:
  /// Power by which to normalize the OpenMC results
  const Real & _power;

  /// Whether to print diagnostic information about model setup and the transfers
  const bool & _verbose;
};
