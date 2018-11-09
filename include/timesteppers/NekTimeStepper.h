#ifndef CARDINAL_NEKTIMESTEPPER_H
#define CARDINAL_NEKTIMESTEPPER_H

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
  virtual void step() override;
  virtual bool converged() override;

private:
  Real _dt;
};
#endif //CARDINAL_NEKTIMESTEPPER_H
