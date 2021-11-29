#pragma once

#include "OpenMCAuxKernel.h"

/**
 * Auxkernel to display the mapping of OpenMC cell temperatures to elements.
 */
class CellTemperatureAux : public OpenMCAuxKernel
{
public:
  CellTemperatureAux(const InputParameters & parameters);

  static InputParameters validParams();

protected:
  virtual Real computeValue();
};
