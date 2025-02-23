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

#include "NekFieldPostprocessor.h"

InputParameters
NekFieldPostprocessor::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  params += NekBase::validParams();
  params += NekFieldInterface::validParams();
  params.addParam<MooseEnum>("mesh", getNekMeshEnum(), "NekRS mesh to compute postprocessor on");
  return params;
}

NekFieldPostprocessor::NekFieldPostprocessor(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    NekBase(this, parameters),
    NekFieldInterface(this, parameters),
    _pp_mesh(getParam<MooseEnum>("mesh").getEnum<nek_mesh::NekMeshEnum>())
{
}

#endif
