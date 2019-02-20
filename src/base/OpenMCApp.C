//
// Created by Ronald Rahaman on 2019-01-23.
//

#include "OpenMCApp.h"
#include "AppFactory.h"
#include "capi.h"

template <>
InputParameters
validParams<OpenMCApp>()
{
  InputParameters params = validParams<MooseApp>();
  return params;
}

OpenMCApp::OpenMCApp(InputParameters parameters) : MooseApp(parameters)
{
  openmc_init(&_communicator.get());
  registerAll(_factory, _action_factory, _syntax);
}

void OpenMCApp::registerAll(Factory &f, ActionFactory &af, Syntax &s)
{
  Registry::registerObjectsTo(f, {"OpenMCApp"});
  Registry::registerActionsTo(af, {"OpenMCApp"});
}

void OpenMCApp::registerApps()
{
  registerApp(OpenMCApp);
}
