#include "CardinalApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "ModulesApp.h"
#include "MooseSyntax.h"
#include "OpenMCSyntax.h"

#ifdef ENABLE_SAM_COUPLING
#include "SamApp.h"
#endif

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
  OpenMC::associateSyntax(s, af);
}

void
CardinalApp::registerApps()
{
  registerApp(CardinalApp);

#ifdef ENABLE_SAM_COUPLING
  registerApp(SamApp);
#endif
}

/***************************************************************************************************
 *********************** Dynamic Library Entry Points - DO NOT MODIFY ******************************
 **************************************************************************************************/
extern "C" void
CardinalApp__registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  CardinalApp::registerAll(f, af, s);

#ifdef ENABLE_SAM_COUPLING
  SamApp::registerAll(f, af, s);
#endif
}
extern "C" void
CardinalApp__registerApps()
{
  CardinalApp::registerApps();
}
