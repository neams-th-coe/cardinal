#ifndef CARDINAL_OPENMCAPP_H
#define CARDINAL_OPENMCAPP_H

#include "MooseApp.h"

class OpenMCApp;

template <>
InputParameters validParams<OpenMCApp>();

class OpenMCApp : public MooseApp
{
public:
  explicit OpenMCApp(InputParameters parameters);
  virtual ~OpenMCApp() {};

  static void registerApps();
  static void registerAll(Factory & f, ActionFactory & af, Syntax & s);
};

#endif //CARDINAL_OPENMCAPP_H
