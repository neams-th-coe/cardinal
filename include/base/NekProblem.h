#ifndef CARDINAL_NEKPROBLEM_H
#define CARDINAL_NEKPROBLEM_H

#include "ExternalProblem.h"

class NekProblem;

template<>
InputParameters validParams<NekProblem>();

class NekProblem : public ExternalProblem
{
public:
  explicit NekProblem(const InputParameters & params);

  virtual void externalSolve() override;
  virtual void syncSolutions(ExternalProblem::Direction direction) override;
};

#endif //CARDINAL_NEKPROBLEM_H
