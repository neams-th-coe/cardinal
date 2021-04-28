#pragma once

#include "OpenMCAuxKernel.h"

class CellTemperatureAux;

template<>
InputParameters validParams<CellTemperatureAux>();

/**
 * Auxkernel to display the mapping of OpenMC cell temperatures to elements.
 */
class CellTemperatureAux : public OpenMCAuxKernel
{
public:
  CellTemperatureAux(const InputParameters & parameters);

protected:
  virtual Real computeValue();
};
