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

#include "NekFunctionNorm.h"
#include "NekInterface.h"

registerMooseObject("CardinalApp", NekFunctionNorm);

InputParameters
NekFunctionNorm::validParams()
{
  InputParameters params = NekPostprocessor::validParams();
  params += NekFieldInterface::validParams();
  params.addRequiredParam<FunctionName>("function", "Function to use for computing the norm");
  params.addRangeCheckedParam<unsigned int>("N", 2, "N>0", "L$^N$ norm to use");
  params.addClassDescription("Integrated L$^N$ norm of a NekRS solution field, relative to a "
                             "provided function, over the NekRS mesh");
  return params;
}

NekFunctionNorm::NekFunctionNorm(const InputParameters & parameters)
  : NekPostprocessor(parameters),
    NekFieldInterface(this, parameters),
    _function(getFunction("function")),
    _N(getParam<unsigned int>("N"))
{
  if (_nek_problem->nondimensional())
    mooseError(
        "The NekFunctionNorm object does not yet support non-dimensional runs! Please contact the "
        "development team to accelerate this feature addition to support your use case.");
}

Real
NekFunctionNorm::getValue() const
{
  return nekrs::functionNorm(_field, _pp_mesh, _function, _t, _N);
}

#endif
