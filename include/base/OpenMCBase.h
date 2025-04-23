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
   * Compute standard deviation of a variable
   * @param[in] mean mean
   * @param[in] sum_sq sum squared
   * @param[in] realizations the number of realizations of the variable
   * @return standard deviation
   */
  Real stdev(const double & mean, const double & sum_sq, unsigned int realizations) const;

  /// The OpenMCCellAverageProblem required by all objects which inherit from OpenMCBase.
  OpenMCCellAverageProblem * _openmc_problem;
};
