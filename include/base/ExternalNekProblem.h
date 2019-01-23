#ifndef CARDINAL_EXTERNALNEKPROBLEM_H
#define CARDINAL_EXTERNALNEKPROBLEM_H

#include "ExternalProblem.h"

class ExternalNekProblem;

template<>
InputParameters validParams<ExternalNekProblem>();

class ExternalNekProblem : public ExternalProblem
{
public:
  explicit ExternalNekProblem(const InputParameters & params);

  virtual void externalSolve() override;
};

#endif //CARDINAL_EXTERNALNEKPROBLEM_H
