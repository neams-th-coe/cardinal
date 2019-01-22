#ifndef CARDINAL_NEKEXECUTIONER_H
#define CARDINAL_NEKEXECUTIONER_H

#include "Transient.h"

class NekExecutioner;

template<>
InputParameters validParams<NekExecutioner>();

class NekExecutioner : public Transient
{
public:
  NekExecutioner(const InputParameters & parameters);

  virtual void init();
  virtual void preStep();
  virtual void postStep();
//  virtual bool lastSolveConverged();
};

#endif //CARDINAL_NEKEXECUTIONER_H
