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

NekApp::NekApp(InputParameters parameters) : MooseApp(parameters)
{
  // Corresponds to cmdOptions struct in nekRS/src/main.cpp
  // auto setup_file = parameters.get<std::string>("nekrs_setup") + ".par";
  // auto size_target = parameters.get<int>("nekrs_buildonly");
  // auto build_only = size_target > 0 ? 1 : 0;
  // auto ci_mode = parameters.get<int>("nekrs_cimode");
  // std::string cache_dir;

  std::string setup_file{"ethier.par"};
  int size_target = 0;
  int build_only = 0;
  int ci_mode = 0;
  std::string cache_dir;

  nekrs::setup(_communicator.get(), build_only, size_target, ci_mode, cache_dir, setup_file);

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
