#pragma once

#include "ExternalProblem.h"
#include "NekTimeStepper.h"

#include <memory>

class NekProblem;

template<>
InputParameters validParams<NekProblem>();

class NekProblem : public ExternalProblem
{
public:
  NekProblem(const InputParameters & params);

  /**
   * \brief Write nekRS's solution at the last output step
   *
   * If Nek is not the master app, the number of time steps it takes is
   * controlled by the master app. Depending on the settings in the `.par` file,
   * it becomes possible that nekRS may not write an output file on the simulation's
   * actual last time step, because Nek may not know when that last time step is.
   * Therefore, here we can force nekRS to write its output.
   **/
  ~NekProblem();

  virtual void initialSetup() override;

  virtual void externalSolve() override;
  virtual void syncSolutions(ExternalProblem::Direction direction) override;

  virtual bool converged() override { return true; }

  virtual void addExternalVariables() override;

  /**
   * \brief Whether nekRS should write an output file for the current time step
   *
   * A nekRS output file (suffix .f000xx) is written if the time step is an integer
   * multiple of the output writing interval or if the time step is the last time step.
   * \return whether to write a nekRS output file
   **/
  virtual bool isOutputStep() const;

protected:
  std::unique_ptr<NumericVector<Number>> _serialized_solution;

  /// The time stepper used for selection of time step size
  NekTimeStepper * _timestepper;

  unsigned int _temp_var;
  unsigned int _avg_flux_var;

  /// Start time of the simulation
  double _start_time;

  /**
   * \brief Interval (in either time or number of time steps), for which to write nekRS output files
   *
   * In nekRS's time stepping loop, they cast this to an integer to get the time step interval on
   * which to write output, so we mimic this here for an easier one-to-one comparison with nekRS's main().
   **/
  double _write_interval;

  /// If nekRS output is controlled by writing on runtime intervals, this represents the next time to write
  double _output_time;

  /// Current simulation time
  double _time;

  /// Current time step index
  int _tstep = 0;
};
