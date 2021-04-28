#pragma once

#include "OpenMCAuxKernel.h"

class CellIDAux;

template<>
InputParameters validParams<CellIDAux>();

/**
 * Auxkernel to display the mapping of OpenMC cell IDs to elements.
 */
class CellIDAux : public OpenMCAuxKernel
{
public:
  CellIDAux(const InputParameters & parameters);

protected:
  virtual Real computeValue();
};
