#pragma once

#include "OpenMCAuxKernel.h"

/**
 * Auxkernel to display the mapping of OpenMC cell densities to MOOSE elements.
 */
class CellDensityAux : public OpenMCAuxKernel
{
public:
  CellDensityAux(const InputParameters & parameters);

  static InputParameters validParams();

protected:
  virtual Real computeValue();
};
