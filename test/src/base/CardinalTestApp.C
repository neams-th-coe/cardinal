/********************************************************************/
/*                  SOFTWARE COPYRIGHT NOTIFICATION                 */
/*                             Cardinal                             */
/*                                                                  */
/*                  (c) 2021 UChicago Argonne, LLC                  */
/*                        ALL RIGHTS RESERVED                       */
/*                                                                  */
/*                 Prepared by UChicago Argonne, LLC                */
/*               Under Contract No. DE-AC02-06CH11357               */
/*                With the U. S. Department of Energy               */
/*                                                                  */
/*             Prepared by Battelle Energy Alliance, LLC            */
/*               Under Contract No. DE-AC07-05ID14517               */
/*                With the U. S. Department of Energy               */
/*                                                                  */
/*                 See LICENSE for full restrictions                */
/********************************************************************/

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

CardinalTestApp::CardinalTestApp(const InputParameters & parameters) : MooseApp(parameters)
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
  registerApp(CardinalTestApp);
  CardinalApp::registerApps();
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
