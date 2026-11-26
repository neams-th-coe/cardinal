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

#include "ConstMonomialAuxKernel.h"

template <typename ComputeValueType>
InputParameters
ConstMonomialAuxKernelTempl<ComputeValueType>::validParams()
{
  return AuxKernelTempl<ComputeValueType>::validParams();
}

template <typename ComputeValueType>
ConstMonomialAuxKernelTempl<ComputeValueType>::ConstMonomialAuxKernelTempl(
    const InputParameters & parameters)
  : AuxKernelTempl<ComputeValueType>(parameters)
{
  if (this->isNodal())
    mooseError("This auxkernel can only be used with elemental variables!");
}

template <typename ComputeValueType>
void
ConstMonomialAuxKernelTempl<ComputeValueType>::errorCoupledConstMonomial(
    const std::string & param_name)
{
  if (this->isCoupled(param_name))
  {
    const auto fe_type = this->getFieldVar(param_name, 0)->feType();
    if (fe_type.family != libMesh::MONOMIAL || fe_type.order != libMesh::CONSTANT)
      this->paramError(param_name,
                       "Only CONSTANT MONOMIAL shape functions are supported. Please "
                       "ensure that '" +
                           param_name + "' is of type MONOMIAL and order CONSTANT.");
  }
  else
    this->paramError(param_name,
                     "The required coupled variable '" + param_name +
                         "' has not been provided! Please ensure '" + param_name +
                         "' is specified in " + this->name() + ".");
}

// Explicitly instantiates the three versions of the ConstMonomialAuxKernelTempl class
template class ConstMonomialAuxKernelTempl<Real>;
template class ConstMonomialAuxKernelTempl<RealVectorValue>;
template class ConstMonomialAuxKernelTempl<RealEigenVector>;
