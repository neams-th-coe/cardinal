//
// Created by Ronald Rahaman on 2019-01-23.
//

#include "NekApp.h"
#include "AppFactory.h"
#include "NekInterface.h"
#include "nekrs.hpp"

registerKnownLabel("NekApp");

template <>
InputParameters
validParams<NekApp>() 
{
  InputParameters params = validParams<MooseApp>();
  params.addCommandLineParam<std::string>(
    "nekrs_setup",  "--nekrs-setup [nekrs_setup]",
    "Specify NekRS setup file (basename for .usr and .rea files)"
  );
  params.addCommandLineParam<int>(
    "nekrs_buildonly",  "--nekrs-buildonly [#procs]",
    0,
    "#procs to build NekRS"
  );
  params.addCommandLineParam<int>(
    "nekrs_cimode",  "--nekrs-cimode [id]",
    0,
    "CI test ID for NekRS"
  );
  return params;
}

NekApp::NekApp(InputParameters parameters) 
  : MooseApp(parameters),
    _setup_file(getParam<std::string>("nekrs_setup")),
    _size_target(getParam<int>("nekrs_buildonly")),
    _build_only(_size_target > 0 ? 1 : 0),
    _ci_mode(getParam<int>("nekrs_cimode"))
{
  std::string cache_dir;

  nekrs::setup(_comm->get(), _build_only, _size_target, _ci_mode, 
      cache_dir, _setup_file);

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
