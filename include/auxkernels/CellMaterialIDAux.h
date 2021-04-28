#pragma once

#include "OpenMCAuxKernel.h"

class CellMaterialIDAux;

template<>
InputParameters validParams<CellMaterialIDAux>();

/**
 * Auxkernel to display the mapping of OpenMC cell material IDs to MOOSE elements.
 */
class CellMaterialIDAux : public OpenMCAuxKernel
{
public:
  CellMaterialIDAux(const InputParameters & parameters);

protected:
  virtual Real computeValue();
};
