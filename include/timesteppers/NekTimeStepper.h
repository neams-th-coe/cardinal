#pragma once

#include "TimeStepper.h"

class NekTimeStepper;

template<>
InputParameters validParams<NekTimeStepper>();

/**
 * \brief Time stepper that reads time step information directly from nekRS
 *
 * This time stepper performs the very simple action of reading stepping
 * information from nekRS. This is necessary for the correct simulation time
 * and time step size to be reflected through the Moose App running nekRS.
 * This class will ignore any constantDT-type time stepping parameters set
 * directly within the [Executioner] block, instead reading all stepping
 * information (start time, end time, number of time steps, and time step
 * size) directly from nekRS data structures. The only situation for which
 * some control can be exerted from the MOOSE side is if NekApp is run as
 * a sub-application, in which case the simulation end time is controlled
 * from the master application.
 *
 * TODO: Once nekRS implements adaptive time stepping (and depending on how
 * it is implemented), we may need to adjust how the total number of time steps
 * and simulation end time are determined throughout the course of the simulation.
 **/
class NekTimeStepper : public TimeStepper
{
public:
  NekTimeStepper(const InputParameters & parameters);

  /**
   * Get the simulation end time
   * \return simulation end time
   **/
  virtual Real getEndTime();

  /**
   * Get the total number of time steps that will be simulated
   * \return number of time steps
   **/
  virtual Real getNumTimeSteps();

protected:
  virtual Real computeInitialDT() override;

  virtual Real computeDT() override;

  /// Total number of time steps to be simulated
  int _num_time_steps;
};
