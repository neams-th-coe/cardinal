#pragma once

#include "TimeStepper.h"

class NekTimeStepper;

template<>
InputParameters validParams<NekTimeStepper>();

class NekTimeStepper : public TimeStepper
{
public:
  NekTimeStepper(const InputParameters & parameters);

protected:
  virtual Real computeInitialDT() override;
  virtual Real computeDT() override;
};
