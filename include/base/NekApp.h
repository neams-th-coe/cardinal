#ifndef CARDINAL_NEKAPP_H
#define CARDINAL_NEKAPP_H

#include "MooseApp.h"

class NekApp;

template <>
InputParameters validParams<NekApp>();

class NekApp : public MooseApp
{
public:
  explicit NekApp(InputParameters parameters);
  virtual ~NekApp() {};

  static void registerApps();
  static void registerAll(Factory & f, ActionFactory & af, Syntax & s);
};

#endif //CARDINAL_NEKAPP_H
