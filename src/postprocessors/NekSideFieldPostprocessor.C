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

#include "NekSideFieldPostprocessor.h"

InputParameters
NekSideFieldPostprocessor::validParams()
{
  InputParameters params = NekSidePostprocessor::validParams();
  params += NekFieldInterface::validParams();
  return params;
}

NekSideFieldPostprocessor::NekSideFieldPostprocessor(const InputParameters & parameters)
  : NekSidePostprocessor(parameters), NekFieldInterface(this, parameters)
{
  if (_function)
    paramError("function", "Providing a shifting function is not yet supported by the NekSpatialBinUserObject derived classes! Please contact the Cardinal developer team to accelerate this feature addition.");
}

#endif
