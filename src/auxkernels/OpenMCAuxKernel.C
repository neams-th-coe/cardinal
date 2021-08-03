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
    mooseError("Auxkernel with name '" + name() + "' can only be used with problems of type 'OpenMCCellAverageProblem'!");

  if (isNodal())
    mooseError("Auxkernel with name '" + name() + "' can only be used with elemental variables!");
}

bool
OpenMCAuxKernel::mappedElement()
{
  OpenMCCellAverageProblem::cellInfo cell_info = _openmc_problem->elemToCellInfo(_current_elem->id());
  return !(cell_info.first == OpenMCCellAverageProblem::UNMAPPED);
}
