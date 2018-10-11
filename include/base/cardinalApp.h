//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
#ifndef CARDINALAPP_H
#define CARDINALAPP_H

#include "MooseApp.h"

class cardinalApp;

template <>
InputParameters validParams<cardinalApp>();

class cardinalApp : public MooseApp
{
public:
  cardinalApp(InputParameters parameters);
  virtual ~cardinalApp();

  static void registerApps();
  static void registerAll(Factory & f, ActionFactory & af, Syntax & s);
};

#endif /* CARDINALAPP_H */
