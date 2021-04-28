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
    mooseError("Auxkernels querying the OpenMC problem can only be used with problems of type 'OpenMCCellAverageProblem'!");

  if (isNodal())
    mooseError("Auxkernels querying the OpenMC problem can only be used with elemental variables!");
}
