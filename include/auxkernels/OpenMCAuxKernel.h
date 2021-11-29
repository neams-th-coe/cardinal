#pragma once

#include "AuxKernel.h"
#include "OpenMCCellAverageProblem.h"

/**
 * Base auxkernel from which to inherit auxkernels that query
 * the OpenMC problem.
 */
class OpenMCAuxKernel : public AuxKernel
{
public:
  OpenMCAuxKernel(const InputParameters & parameters);

  static InputParameters validParams();

protected:
  virtual Real computeValue() = 0;

  /**
   * Determine whether the MOOSE element maps to an OpenMC cell to make sure we don't call
   * accessor methods that rely on valid values for the cell instance and index
   * @return whether element maps to OpenMC
   */
  bool mappedElement();

  OpenMCCellAverageProblem * _openmc_problem;
};
