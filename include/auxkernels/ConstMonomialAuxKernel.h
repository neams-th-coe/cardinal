#pragma once

#include "AuxKernel.h"

// forward declarations
template <typename ComputeValueType>
class ConstMonomialAuxKernelTempl;

typedef ConstMonomialAuxKernelTempl<Real> ConstMonomialAuxKernel;
typedef ConstMonomialAuxKernelTempl<RealVectorValue> ConstMonomialVectorAuxKernel;
typedef ConstMonomialAuxKernelTempl<RealEigenVector> ConstMonomialArrayAuxKernel;

/**
 * A base auxkernel that is restricted to constant monomial field variables.
 */
template <typename ComputeValueType>
class ConstMonomialAuxKernelTempl : public AuxKernelTempl<ComputeValueType>
{
public:
  ConstMonomialAuxKernelTempl(const InputParameters & parameters);

  static InputParameters validParams();

protected:
  virtual ComputeValueType computeValue() = 0;

  /**
   * A function which throws an error if a coupled variable is not a constant monomial.
   * @param[in] param_name the name of the paraemter to check
   */
  void errorCoupledConstMonomial(const std::string & param_name);
};

// Prevent implicit instantiation in other translation units where these classes are used
extern template class ConstMonomialAuxKernelTempl<Real>;
extern template class ConstMonomialAuxKernelTempl<RealVectorValue>;
extern template class ConstMonomialAuxKernelTempl<RealEigenVector>;
