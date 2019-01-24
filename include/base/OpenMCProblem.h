#ifndef CARDINAL_OPENMCPROBLEM_H
#define CARDINAL_OPENMCPROBLEM_H

#include "ExternalProblem.h"

class OpenMCProblem;

template<>
InputParameters validParams<OpenMCProblem>();

class OpenMCProblem : public ExternalProblem
{
public:
  explicit OpenMCProblem(const InputParameters & params);

  virtual void externalSolve() override;
  virtual void syncSolutions(ExternalProblem::Direction direction) override;
};


#endif //CARDINAL_OPENMCPROBLEM_H
