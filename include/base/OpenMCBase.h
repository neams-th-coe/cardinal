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

#include "OpenMCCellAverageProblem.h"

namespace libMesh
{
  class Elem;
}

class OpenMCBase
{
public:
  static InputParameters validParams();

  OpenMCBase(const MooseObject * moose_object, const InputParameters & parameters);

protected:
  /**
   * Determine whether the MOOSE element maps to an OpenMC cell to make sure we don't call
   * accessor methods that rely on valid values for the cell instance and index
   * @return whether element maps to OpenMC
   */
  bool mappedElement(const Elem * elem) const;

  /**
   * Get the variable(s) associated with an OpenMC tally score.
   * @param[in] score the OpenMC score
   * @return a vector of variable values associated with score
   */
  std::vector<const MooseVariableFE<Real> *> getTallyScoreVariables(const std::string & score, THREAD_ID t_id) const;

  /**
   * Get the variable value(s) associated with an OpenMC tally score.
   * @param[in] score the OpenMC score
   * @return a vector of variable values associated with score
   */
  std::vector<const VariableValue *> getTallyScoreVariableValues(const std::string & score, THREAD_ID t_id) const;

  /**
   * Get the neighboring variable value(s) associated with an OpenMC tally score.
   * @param[in] score the OpenMC score
   * @return a vector of variable values associated with score
   */
  std::vector<const VariableValue *> getTallyScoreNeighborVariableValues(const std::string & score, THREAD_ID t_id) const;

  /// The OpenMCCellAverageProblem required by all objects which inherit from OpenMCBase.
  const OpenMCCellAverageProblem * _openmc_problem;
};
