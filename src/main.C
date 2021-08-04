//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
#include "MooseInit.h"
#include "Moose.h"
#include "MooseApp.h"
#include "AppFactory.h"
#include "CommandLine.h"
#include "CardinalApp.h"

// Create a performance log
PerfLog Moose::perf_log("cardinal");

// Begin the main program.
int
main(int argc, char *argv[])
{
  // Initialize MPI, solvers and MOOSE
  MooseInit init(argc, argv);

  // Get which app to run
  CommandLine::Option app_opt = {"which app to run", {"--app", "app_name"}, false, CommandLine::ARGUMENT::OPTIONAL, {}};

  auto cmds = CommandLine(argc, argv);
  cmds.addOption("which_app", app_opt);

  std::string which_app;
  cmds.search("which_app", which_app);

  CardinalApp::registerApps();

  std::string app_class_name;
  if (which_app == "openmc")
    mooseError("You no longer need to specify '--app openmc' for wrapped OpenMC inputs.\n"
      "The correct app is now inferred from the [Problem] block in the input file.\n"
      "Please simply remove the '--app openmc' from your run command.");

  if (which_app == "nek5000" or which_app == "nek")
    mooseError("You no longer need to specify '--app nek' or '--app nek5000' for wrapped Nek inputs.\n"
      "The correct app is now inferred from the [Problem] block in the input file.\n"
      "Please simply remove the '--app nek'/'--app nek5000' from your run command.");

  if (which_app == "sam")
    app_class_name = "SamApp";
  else
    app_class_name = "CardinalApp";

  // Create and run the app
  std::shared_ptr<MooseApp> app = AppFactory::createAppShared(app_class_name, argc, argv);
  app->run();

  return 0;
}
