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

#include "OpenMCAuxKernel.h"

template<>
InputParameters validParams<OpenMCAuxKernel>()
{
  InputParameters params = validParams<AuxKernel>();
  return params;
}

OpenMCAuxKernel::OpenMCAuxKernel(const InputParameters & parameters) :
    AuxKernel(parameters)
{
  _openmc_problem = dynamic_cast<OpenMCCellAverageProblem *>(&_subproblem);

  if (!_openmc_problem)
    mooseError("This auxkernel can only be used with problems of type 'OpenMCCellAverageProblem'!");

  if (isNodal())
    mooseError("This auxkernel can only be used with elemental variables!");
}

bool
OpenMCAuxKernel::mappedElement()
{
  OpenMCCellAverageProblem::cellInfo cell_info = _openmc_problem->elemToCellInfo(_current_elem->id());
  return !(cell_info.first == OpenMCCellAverageProblem::UNMAPPED);
}
