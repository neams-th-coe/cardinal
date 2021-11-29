#pragma once

#include "OpenMCAuxKernel.h"
#include "OpenMCCellAverageProblem.h"

/**
 * Auxkernel to display the mapping of OpenMC cell instances to elements.
 */
class CellInstanceAux : public OpenMCAuxKernel
{
public:
  CellInstanceAux(const InputParameters & parameters);

  static InputParameters validParams();

protected:
  virtual Real computeValue();
};
