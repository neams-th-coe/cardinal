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
