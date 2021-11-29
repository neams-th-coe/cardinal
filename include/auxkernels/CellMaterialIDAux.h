#pragma once

#include "OpenMCAuxKernel.h"

/**
 * Auxkernel to display the mapping of OpenMC cell material IDs to MOOSE elements.
 */
class CellMaterialIDAux : public OpenMCAuxKernel
{
public:
  CellMaterialIDAux(const InputParameters & parameters);

  static InputParameters validParams();

protected:
  virtual Real computeValue();
};
