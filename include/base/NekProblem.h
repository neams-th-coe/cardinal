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
  ~NekProblem(){}

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

  /// Interval, in number of time steps, for which to write nekRS output files
  int _outputStep;

  /// Current simulation time
  double _time;

  /// Current time step index
  int _tstep = 0;
};
