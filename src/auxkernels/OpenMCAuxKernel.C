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

#ifdef ENABLE_OPENMC_COUPLING

#include "OpenMCAuxKernel.h"

template <typename ComputeValueType>
InputParameters
OpenMCAuxKernelTempl<ComputeValueType>::validParams()
{
  InputParameters params = AuxKernelTempl<ComputeValueType>::validParams();
  params += TallyInterface::validParams();
  return params;
}

template <typename ComputeValueType>
OpenMCAuxKernelTempl<ComputeValueType>::OpenMCAuxKernelTempl(const InputParameters & parameters)
  : AuxKernelTempl<ComputeValueType>(parameters), TallyInterface(this, parameters)
{
  if (this->isNodal())
    mooseError("This auxkernel can only be used with elemental variables!");
}

template <typename ComputeValueType>
bool
OpenMCAuxKernelTempl<ComputeValueType>::mappedElement()
{
  OpenMCCellAverageProblem::cellInfo cell_info =
      _openmc_problem->elemToCellInfo(this->_current_elem->id());
  return !(cell_info.first == OpenMCCellAverageProblem::UNMAPPED);
}


template <typename ComputeValueType>
void
OpenMCAuxKernelTempl<ComputeValueType>::errorCoupledConstMonomial(const std::string & param_name)
{
  if (this->isCoupled(param_name))
    if (this->getFieldVar(param_name, 0)->feType() != FEType(libMesh::CONSTANT, libMesh::MONOMIAL))
      this->paramError(param_name,
                       this->name() + " only supports CONSTANT MONOMIAL shape functions. Please "
                       "ensure that '" + param_name + "' is of type MONOMIAL and order CONSTANT.");
}

// Explicitly instantiates the three versions of the OpenMCAuxKernelTempl class
template class OpenMCAuxKernelTempl<Real>;
template class OpenMCAuxKernelTempl<RealVectorValue>;
template class OpenMCAuxKernelTempl<RealEigenVector>;

#endif
