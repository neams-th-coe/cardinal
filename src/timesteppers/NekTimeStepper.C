#include "NekTimeStepper.h"
#include "nekrs.hpp"

template<>
InputParameters validParams<NekTimeStepper>()
{
  InputParameters params = validParams<TimeStepper>();
  return params;
}

NekTimeStepper::NekTimeStepper(const InputParameters & parameters) :
    TimeStepper(parameters)
{
}

Real
NekTimeStepper::computeInitialDT()
{
  return nekrs::dt();
}

Real
NekTimeStepper::computeDT()
{
  return nekrs::dt();
}
