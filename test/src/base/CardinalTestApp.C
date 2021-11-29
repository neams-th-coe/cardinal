//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
#include "CardinalTestApp.h"
#include "CardinalApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"
#include "ModulesApp.h"

InputParameters
CardinalTestApp::validParams()
{
  InputParameters params = CardinalApp::validParams();
  return params;
}

CardinalTestApp::CardinalTestApp(InputParameters parameters) : MooseApp(parameters)
{
  CardinalTestApp::registerAll(
      _factory, _action_factory, _syntax, getParam<bool>("allow_test_objects"));
}

CardinalTestApp::~CardinalTestApp() {}

void
CardinalTestApp::registerAll(Factory & f, ActionFactory & af, Syntax & s, bool use_test_objs)
{
  CardinalApp::registerAll(f, af, s);
  if (use_test_objs)
  {
    Registry::registerObjectsTo(f, {"CardinalTestApp"});
    Registry::registerActionsTo(af, {"CardinalTestApp"});
  }
}

void
CardinalTestApp::registerApps()
{
  registerApp(CardinalApp);
  registerApp(CardinalTestApp);
}

/***************************************************************************************************
 *********************** Dynamic Library Entry Points - DO NOT MODIFY ******************************
 **************************************************************************************************/
// External entry point for dynamic application loading
extern "C" void
CardinalTestApp__registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  CardinalTestApp::registerAll(f, af, s);
}
extern "C" void
CardinalTestApp__registerApps()
{
  CardinalTestApp::registerApps();
}
