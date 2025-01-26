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
  /// The OpenMCCellAverageProblem required by all objects which inherit from OpenMCBase.
  OpenMCCellAverageProblem * _openmc_problem;
};
