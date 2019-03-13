#include "cardinalApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "ModulesApp.h"
#include "MooseSyntax.h"

template <>
InputParameters
validParams<cardinalApp>()
{
  InputParameters params = validParams<MooseApp>();
  return params;
}

cardinalApp::cardinalApp(InputParameters parameters) : MooseApp(parameters)
{
  cardinalApp::registerAll(_factory, _action_factory, _syntax);
}

cardinalApp::~cardinalApp() {}

void
cardinalApp::registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  ModulesApp::registerAll(f, af, s);
  Registry::registerObjectsTo(f, {"cardinalApp"});
  Registry::registerActionsTo(af, {"cardinalApp"});

  /* register custom execute flags, action syntax, etc. here */
}

void
cardinalApp::registerApps()
{
  registerApp(cardinalApp);
}

/***************************************************************************************************
 *********************** Dynamic Library Entry Points - DO NOT MODIFY ******************************
 **************************************************************************************************/
extern "C" void
cardinalApp__registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  cardinalApp::registerAll(f, af, s);
}
extern "C" void
cardinalApp__registerApps()
{
  cardinalApp::registerApps();
}
