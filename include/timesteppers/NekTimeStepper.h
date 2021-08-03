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
 * some control can be exerted from the MOOSE side is if Nek is run as
 * a sub-application, in which case the simulation end time is controlled
 * from the master application.
 **/
class NekTimeStepper : public TimeStepper
{
public:
  NekTimeStepper(const InputParameters & parameters);

  virtual Real minDT() const;

  /**
   * Set the reference time scale that nekRS's nondimensional form is based on
   * @param[in] L characteristic length
   * @param[in] U characteristic velocity
   */
  virtual void setReferenceTime(const Real & L, const Real & U);

  /**
   * Compute the nondimensional version of a dt
   * @param[in] dimensional_dt time step in dimensional form
   * @return non-dimensional version of time step
   */
  virtual Real nondimensionalDT(const Real & dimensional_dt) const;

  /**
   * Compute the dimensional version of a dt
   * @param[in] nondimensional_dt time step in nondimensional form
   * @return dimensional version of time step
   */
  virtual Real dimensionalDT(const Real & nondimensional_dt) const;

  /// Dimensionalize the time step for nekRS
  virtual void dimensionalizeDT();

protected:
  virtual Real computeInitialDT() override;

  virtual Real computeDT() override;

  Real _min_dt;

  Real _nek_dt;

  /// Reference time scale
  Real _t_ref;
};
