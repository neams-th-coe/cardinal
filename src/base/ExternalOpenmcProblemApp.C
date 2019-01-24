//
// Created by Ronald Rahaman on 2019-01-23.
//

#include "ExternalOpenmcProblemApp.h"
#include "AppFactory.h"
#include "openmc.h"

template <>
InputParameters
validParams<ExternalOpenmcProblemApp>()
{
  InputParameters params = validParams<MooseApp>();
  return params;
}

ExternalOpenmcProblemApp::ExternalOpenmcProblemApp(InputParameters parameters) : MooseApp(parameters)
{
  openmc_init(&_communicator.get());
  registerAll(_factory, _action_factory, _syntax);
}

void ExternalOpenmcProblemApp::registerAll(Factory &f, ActionFactory &af, Syntax &s)
{
  Registry::registerObjectsTo(f, {"ExternalOpenmcProblemApp"});
  Registry::registerActionsTo(af, {"ExternalOpenmcProblemApp"});
}

void ExternalOpenmcProblemApp::registerApps()
{
  registerApp(ExternalOpenmcProblemApp);
}
