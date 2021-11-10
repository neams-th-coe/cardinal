#include "OpenMCPostprocessor.h"

defineLegacyParams(OpenMCPostprocessor);

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
    mooseError("Postprocessor with name '" + name() + "' can only be used with OpenMCCellAverageProblem!");
}
