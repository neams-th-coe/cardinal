#include "CardinalApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "ModulesApp.h"
#include "MooseSyntax.h"

registerKnownLabel("CardinalApp");

template <>
InputParameters
validParams<CardinalApp>()
{
  InputParameters params = validParams<MooseApp>();
  return params;
}

CardinalApp::CardinalApp(InputParameters parameters) : MooseApp(parameters)
{
  CardinalApp::registerAll(_factory, _action_factory, _syntax);
}

CardinalApp::~CardinalApp() {}

void
CardinalApp::registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  ModulesApp::registerAll(f, af, s);
  Registry::registerObjectsTo(f, {"CardinalApp"});
  Registry::registerActionsTo(af, {"CardinalApp"});

  /* register custom execute flags, action syntax, etc. here */
}

void
CardinalApp::registerApps()
{
  registerApp(CardinalApp);
}

/***************************************************************************************************
 *********************** Dynamic Library Entry Points - DO NOT MODIFY ******************************
 **************************************************************************************************/
extern "C" void
CardinalApp__registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  CardinalApp::registerAll(f, af, s);
}
extern "C" void
CardinalApp__registerApps()
{
  CardinalApp::registerApps();
}
