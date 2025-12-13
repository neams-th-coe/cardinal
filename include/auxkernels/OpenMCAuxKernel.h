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

#pragma once

#include "AuxKernel.h"
#include "TallyInterface.h"
#include "OpenMCCellAverageProblem.h"

// forward declarations
template <typename ComputeValueType>
class OpenMCAuxKernelTempl;

typedef OpenMCAuxKernelTempl<Real> OpenMCAuxKernel;
typedef OpenMCAuxKernelTempl<RealVectorValue> OpenMCVectorAuxKernel;
typedef OpenMCAuxKernelTempl<RealEigenVector> OpenMCArrayAuxKernel;

/**
 * Base auxkernel from which to inherit auxkernels that query
 * the OpenMC problem.
 */
template <typename ComputeValueType>
class OpenMCAuxKernelTempl : public AuxKernelTempl<ComputeValueType>, public TallyInterface
{
public:
  OpenMCAuxKernelTempl(const InputParameters & parameters);

  static InputParameters validParams();

protected:
  virtual ComputeValueType computeValue() = 0;

  /**
   * Determine whether the MOOSE element maps to an OpenMC cell to make sure we don't call
   * accessor methods that rely on valid values for the cell instance and index
   * @return whether element maps to OpenMC
   */
  bool mappedElement();
};

// Prevent implicit instantiation in other translation units where these classes are used
extern template class OpenMCAuxKernelTempl<Real>;
extern template class OpenMCAuxKernelTempl<RealVectorValue>;
extern template class OpenMCAuxKernelTempl<RealEigenVector>;
