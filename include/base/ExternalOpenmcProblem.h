#ifndef CARDINAL_EXTERNALOPENMCPROBLEM_H
#define CARDINAL_EXTERNALOPENMCPROBLEM_H

#include "ExternalProblem.h"

class ExternalOpenmcProblem;

template<>
InputParameters validParams<ExternalOpenmcProblem>();

class ExternalOpenmcProblem : public ExternalProblem
{
public:
  explicit ExternalOpenmcProblem(const InputParameters & params);

  virtual void externalSolve() override;
  virtual void syncSolutions(ExternalProblem::Direction direction) override;
};


#endif //CARDINAL_EXTERNALOPENMCPROBLEM_H
