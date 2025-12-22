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

// Explicitly instantiates the three versions of the OpenMCAuxKernelTempl class
template class OpenMCAuxKernelTempl<Real>;
template class OpenMCAuxKernelTempl<RealVectorValue>;
template class OpenMCAuxKernelTempl<RealEigenVector>;

#endif
