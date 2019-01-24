//
// Created by Ronald Rahaman on 2019-01-23.
//

#include "NekApp.h"
#include "AppFactory.h"
#include "openmc.h"

template <>
InputParameters
validParams<NekApp>()
{
  InputParameters params = validParams<MooseApp>();
  return params;
}

NekApp::NekApp(InputParameters parameters) : MooseApp(parameters)
{
  openmc_init(&_communicator.get());
  registerAll(_factory, _action_factory, _syntax);
}

void NekApp::registerAll(Factory &f, ActionFactory &af, Syntax &/*s*/)
{
  Registry::registerObjectsTo(f, {"NekApp"});
  Registry::registerActionsTo(af, {"NekApp"});
}

void NekApp::registerApps()
{
  registerApp(NekApp);
}
