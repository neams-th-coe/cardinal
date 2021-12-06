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

#include "OpenMCPostprocessor.h"

InputParameters
OpenMCPostprocessor::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  return params;
}

OpenMCPostprocessor::OpenMCPostprocessor(const InputParameters & parameters) :
  GeneralPostprocessor(parameters)
{
  _openmc_problem = dynamic_cast<OpenMCCellAverageProblem *>(&_fe_problem);

  if (!_openmc_problem)
    mooseError("This postprocessor can only be used with OpenMCCellAverageProblem!");
}
