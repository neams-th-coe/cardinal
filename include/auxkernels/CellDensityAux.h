#pragma once

#include "OpenMCAuxKernel.h"

class CellDensityAux;

template<>
InputParameters validParams<CellDensityAux>();

/**
 * Auxkernel to display the mapping of OpenMC cell densities to MOOSE elements.
 */
class CellDensityAux : public OpenMCAuxKernel
{
public:
  CellDensityAux(const InputParameters & parameters);

protected:
  virtual Real computeValue();
};
