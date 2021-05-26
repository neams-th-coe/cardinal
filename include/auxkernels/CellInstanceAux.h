#pragma once

#include "OpenMCAuxKernel.h"
#include "OpenMCCellAverageProblem.h"

class CellInstanceAux;

template<>
InputParameters validParams<CellInstanceAux>();

/**
 * Auxkernel to display the mapping of OpenMC cell instances to elements.
 */
class CellInstanceAux : public OpenMCAuxKernel
{
public:
  CellInstanceAux(const InputParameters & parameters);

protected:
  virtual Real computeValue();
};
