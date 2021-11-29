#pragma once

#include "OpenMCAuxKernel.h"

/**
 * Auxkernel to display the mapping of OpenMC cell IDs to elements.
 */
class CellIDAux : public OpenMCAuxKernel
{
public:
  CellIDAux(const InputParameters & parameters);

  static InputParameters validParams();

protected:
  virtual Real computeValue();
};
