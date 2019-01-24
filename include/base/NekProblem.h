#ifndef CARDINAL_NEKPROBLEM_H
#define CARDINAL_NEKPROBLEM_H

#include "ExternalProblem.h"

class NekProblem;

template<>
InputParameters validParams<NekProblem>();

class NekProblem : public ExternalProblem
{
public:
  NekProblem(const InputParameters & params);
  ~NekProblem(){}

  virtual void externalSolve() override;
  virtual void syncSolutions(ExternalProblem::Direction direction) override;

  virtual bool converged() override { return true; }
};

#endif //CARDINAL_NEKPROBLEM_H
