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
      cache_dir, _setup_file, "", "");

  // First check we should do is that a temperature variable exists, or else many
  // of our indexes into `nrs->cds` would give seg faults
  if (!nekrs::hasTemperatureVariable())
    mooseError("To properly transfer temperature and heat flux between nekRS and MOOSE, "
      "your nekRS model must include a solution for temperature.\n\nDid you forget the "
      "TEMPERATURE block in the .par file?");

  // Allocate space for user scratch space on both the host and device. nekRS
  // does not have a dedicated array to store a "flux" boundary condition, so we
  // will make use of these arrays to write incoming flux values from MOOSE.
  bool scratch_available = nekrs::initializeScratch();

  if (!scratch_available)
    mooseError("The nrs_t.usrwrk and nrs_t.o_usrwrk arrays are reserved in Cardinal "
      "for transferring the heat flux fom MOOSE to nekRS!\n\nThis memory allocation is done "
      "automatically, but you have already initialized these arrays, perhaps for a different "
      "purpose than transferring flux. Please remove "
      "any UDF usage of these arrays to continue.");

  registerAll(_factory, _action_factory, _syntax);
}

NekApp::~NekApp()
{
  nekrs::freeScratch();
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
