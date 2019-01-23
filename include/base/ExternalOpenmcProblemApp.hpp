#ifndef CARDINAL_EXTERNALOPENMCPROBLEMAPP_H
#define CARDINAL_EXTERNALOPENMCPROBLEMAPP_H

#include "MooseApp.h"

class ExternalOpenmcProblemApp;

template <>
InputParameters validParams<ExternalOpenmcProblemApp>();

class ExternalOpenmcProblemApp : public MooseApp
{
public:
  explicit ExternalOpenmcProblemApp(InputParameters parameters);
  virtual ~ExternalOpenmcProblemApp() {};

  static void registerApps();
  static void registerAll(Factory & f, ActionFactory & af, Syntax & s);
};

#endif //CARDINAL_EXTERNALOPENMCPROBLEMAPP_H
