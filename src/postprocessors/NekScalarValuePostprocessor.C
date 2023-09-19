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

#include "NekScalarValuePostprocessor.h"

registerMooseObject("CardinalApp", NekScalarValuePostprocessor);

InputParameters
NekScalarValuePostprocessor::validParams()
{
  InputParameters params = NekPostprocessor::validParams();
  params.addRequiredParam<UserObjectName>("userobject", "NekScalarValue userobject to report value for");
  params.suppressParameter<MooseEnum>("mesh");
  params.addClassDescription("Reports the current value held by a NekScalarValue userobject");
  return params;
}

NekScalarValuePostprocessor::NekScalarValuePostprocessor(const InputParameters & parameters)
  : NekPostprocessor(parameters),
    _uo(getUserObject<NekScalarValue>("userobject"))
{
}

Real
NekScalarValuePostprocessor::getValue() const
{
  return _uo.getValue();
}

#endif
