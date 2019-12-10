//
// Created by Ronald Rahaman on 2019-01-23.
//

#include "NekApp.h"
#include "AppFactory.h"
#include "NekInterface.h"

registerKnownLabel("NekApp");

template <>
InputParameters
validParams<NekApp>()
{
  InputParameters params = validParams<MooseApp>();
  return params;
}

NekApp::NekApp(InputParameters parameters) : MooseApp(parameters)
{
  /*
  Nek5000::FORTRAN_CALL(nek_init)(_communicator.get());
  */

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
