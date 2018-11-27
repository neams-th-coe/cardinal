#ifndef CARDINAL_COUPLEDGRADAUX_H
#define CARDINAL_COUPLEDGRADAUX_H

#include "AuxKernel.h"


//Forward Declarations
class CoupledGradAux;

template<>
InputParameters validParams<CoupledGradAux>();

/**
 * Coupled auxiliary gradient
 */
class CoupledGradAux : public AuxKernel
{
public:

  /**
   * Factory constructor, takes parameters so that all derived classes can be built using the same
   * constructor.
   */
  CoupledGradAux(const InputParameters & parameters);

  virtual ~CoupledGradAux();

protected:
  virtual Real computeValue();

  /// Gradient being set by this kernel
  RealGradient _grad;
  /// The number of coupled variable
  int _coupled;
  /// The value of coupled gradient
  const VariableGradient & _coupled_grad;
  /// Thermal conductivity (diffusion coefficient)
  const MaterialProperty<Real> & _diffusion_coefficient;
};

#endif //CARDINAL_COUPLEDGRADAUX_H
