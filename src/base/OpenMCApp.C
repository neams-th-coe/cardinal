//
// Created by Ronald Rahaman on 2019-01-23.
//

#include "OpenMCApp.h"
#include "AppFactory.h"
#include "openmc/capi.h"

registerKnownLabel("OpenMCApp");

template <>
InputParameters
validParams<OpenMCApp>()
{
  InputParameters params = validParams<MooseApp>();
  return params;
}

OpenMCApp::OpenMCApp(InputParameters parameters) : MooseApp(parameters)
{
  int argc = 1;
  char openmc[] = "openmc";
  char * argv[1] = { openmc };

  openmc_init(argc, argv, &_communicator.get());
  registerAll(_factory, _action_factory, _syntax);
}

void OpenMCApp::registerAll(Factory &f, ActionFactory &af, Syntax &/*s*/)
{
  Registry::registerObjectsTo(f, {"OpenMCApp"});
  Registry::registerActionsTo(af, {"OpenMCApp"});
}

void OpenMCApp::registerApps()
{
  registerApp(OpenMCApp);
}

OpenMCApp::~OpenMCApp() 
{
  _console << "Finalizing OpenMC...";
  openmc_finalize();
}
