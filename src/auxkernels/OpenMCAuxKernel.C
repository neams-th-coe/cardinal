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

#ifdef ENABLE_OPENMC_COUPLING

#include "OpenMCAuxKernel.h"

InputParameters
OpenMCAuxKernel::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params += OpenMCBase::validParams();
  return params;
}

OpenMCAuxKernel::OpenMCAuxKernel(const InputParameters & parameters) : AuxKernel(parameters), OpenMCBase(this, parameters)
{
  if (isNodal())
    mooseError("This auxkernel can only be used with elemental variables!");
}

bool
OpenMCAuxKernel::mappedElement()
{
  OpenMCCellAverageProblem::cellInfo cell_info =
      _openmc_problem->elemToCellInfo(_current_elem->id());
  return !(cell_info.first == OpenMCCellAverageProblem::UNMAPPED);
}

#endif
