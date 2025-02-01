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

#ifdef ENABLE_NEK_COUPLING

#include "NekFunctionL2Norm.h"
#include "NekInterface.h"

registerMooseObject("CardinalApp", NekFunctionL2Norm);

InputParameters
NekFunctionL2Norm::validParams()
{
  InputParameters params = NekFieldPostprocessor::validParams();
  params.addRequiredParam<FunctionName>("function", "Function to use for computing the norm");
  params.addClassDescription("Integrated L2 norm of a NekRS solution field, relative to a provided function, over the NekRS mesh");
  return params;
}

NekFunctionL2Norm::NekFunctionL2Norm(const InputParameters & parameters)
  : NekFieldPostprocessor(parameters),
    _function(getFunction("function"))
{
  if (_nek_problem->nondimensional())
    mooseError("The NekFunctionL2Norm object does not yet support non-dimensional runs! Please contact the development team to accelerate this feature addition to support your use case.");
}

Real
NekFunctionL2Norm::getValue() const
{
  return nekrs::functionL2Norm(_field, _pp_mesh, _function, _t);
}

#endif
