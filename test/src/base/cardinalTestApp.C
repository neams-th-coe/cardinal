//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
#include "cardinalTestApp.h"
#include "cardinalApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"
#include "ModulesApp.h"

template <>
InputParameters
validParams<cardinalTestApp>()
{
  InputParameters params = validParams<cardinalApp>();
  return params;
}

cardinalTestApp::cardinalTestApp(InputParameters parameters) : MooseApp(parameters)
{
  cardinalTestApp::registerAll(
      _factory, _action_factory, _syntax, getParam<bool>("allow_test_objects"));
}

cardinalTestApp::~cardinalTestApp() {}

void
cardinalTestApp::registerAll(Factory & f, ActionFactory & af, Syntax & s, bool use_test_objs)
{
  cardinalApp::registerAll(f, af, s);
  if (use_test_objs)
  {
    Registry::registerObjectsTo(f, {"cardinalTestApp"});
    Registry::registerActionsTo(af, {"cardinalTestApp"});
  }
}

void
cardinalTestApp::registerApps()
{
  registerApp(cardinalApp);
  registerApp(cardinalTestApp);
}

/***************************************************************************************************
 *********************** Dynamic Library Entry Points - DO NOT MODIFY ******************************
 **************************************************************************************************/
// External entry point for dynamic application loading
extern "C" void
cardinalTestApp__registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  cardinalTestApp::registerAll(f, af, s);
}
extern "C" void
cardinalTestApp__registerApps()
{
  cardinalTestApp::registerApps();
}
