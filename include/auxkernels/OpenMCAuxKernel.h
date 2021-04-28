#pragma once

#include "AuxKernel.h"
#include "OpenMCCellAverageProblem.h"

class OpenMCAuxKernel;

template<>
InputParameters validParams<OpenMCAuxKernel>();

/**
 * Base auxkernel from which to inherit auxkernels that query
 * the OpenMC problem.
 */
class OpenMCAuxKernel : public AuxKernel
{
public:
  OpenMCAuxKernel(const InputParameters & parameters);

protected:
  virtual Real computeValue() = 0;

  OpenMCCellAverageProblem * _openmc_problem;
};
