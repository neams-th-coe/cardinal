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

#include "CardinalApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "ModulesApp.h"
#include "MooseSyntax.h"
#include "CardinalAppTypes.h"
#include "CardinalRevision.h"

#ifdef ENABLE_NEK_COUPLING
#include "NekSyntax.h"
#endif

#ifdef ENABLE_GRIFFIN_COUPLING
#include "GriffinApp.h"
#include "RadiationTransportApp.h"
#endif

#ifdef ENABLE_BISON_COUPLING
#include "BisonApp.h"
#endif

#ifdef ENABLE_SAM_COUPLING
#include "SamApp.h"
#endif

#ifdef ENABLE_SOCKEYE_COUPLING
#include "SockeyeApp.h"
#endif

#ifdef ENABLE_SODIUM
#include "SodiumApp.h"
#endif

#ifdef ENABLE_POTASSIUM
#include "PotassiumApp.h"
#endif

#ifdef ENABLE_IAPWS95
#include "IAPWS95App.h"
#endif

registerKnownLabel("CardinalApp");

InputParameters
CardinalApp::validParams()
{
  InputParameters params = MooseApp::validParams();

  // only used for Nek wrappings - if used with another application (OpenMC wrapping
  // or just plain MOOSE-type apps), these are unused
  params.addCommandLineParam<int>(
      "nekrs_buildonly", "--nekrs-buildonly [#procs]", "#procs to build NekRS if pre-compiling");
  params.addCommandLineParam<int>("nekrs_cimode",
                                  "--nekrs-cimode [id]",
                                  "Test ID for NekRS CI settings for execution within Cardinal");
  params.addCommandLineParam<std::string>(
      "nekrs_backend",
      "--nekrs-backend",
      "Backend to use for NekRS parallelism; options: CPU, CUDA, HIP, OPENCL, OPENMP");
  params.addCommandLineParam<std::string>(
      "nekrs_device_id", "--nekrs-device-id", "NekRS device ID");

  params.set<bool>("use_legacy_material_output") = false;
  params.set<bool>("use_legacy_initial_residual_evaluation_behavior") = false;
  params.set<bool>("error_unused") = false;
  params.set<bool>("allow_unused") = true;
  return params;
}

CardinalApp::CardinalApp(InputParameters parameters) : MooseApp(parameters)
{
  CardinalApp::registerAll(_factory, _action_factory, _syntax);
}

void
CardinalApp::registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  Registry::registerObjectsTo(f, {"CardinalApp"});
  Registry::registerActionsTo(af, {"CardinalApp"});

  ModulesApp::registerAllObjects<CardinalApp>(f, af, s);

#ifdef ENABLE_GRIFFIN_COUPLING
  GriffinApp::registerAll(f, af, s);
  RadiationTransportApp::registerAll(f, af, s);
#endif

#ifdef ENABLE_BISON_COUPLING
  BisonApp::registerAll(f, af, s);
#endif

#ifdef ENABLE_SAM_COUPLING
  SamApp::registerAll(f, af, s);
#endif

#ifdef ENABLE_SOCKEYE_COUPLING
  SockeyeApp::registerAll(f, af, s);
#endif

#ifdef ENABLE_SODIUM
  SodiumApp::registerAll(f, af, s);
#endif

#ifdef ENABLE_POTASSIUM
  PotassiumApp::registerAll(f, af, s);
#endif

#ifdef ENABLE_IAPWS95
  IAPWS95App::registerAll(f, af, s);
#endif

  /* register custom execute flags, action syntax, etc. here */
#ifdef ENABLE_NEK_COUPLING
  Nek::associateSyntax(s, af);
#endif

  associateSyntaxInner(s, af);
}

void
CardinalApp::registerApps()
{
  registerApp(CardinalApp);

  ModulesApp::registerApps();

#ifdef ENABLE_GRIFFIN_COUPLING
  GriffinApp::registerApps();
  RadiationTransportApp::registerApps();
#endif

#ifdef ENABLE_BISON_COUPLING
  BisonApp::registerApps();
#endif

#ifdef ENABLE_SAM_COUPLING
  SamApp::registerApps();
#endif

#ifdef ENABLE_SOCKEYE_COUPLING
  SockeyeApp::registerApps();
#endif

#ifdef ENABLE_SODIUM
  SodiumApp::registerApps();
#endif

#ifdef ENABLE_POTASSIUM
  PotassiumApp::registerApps();
#endif

#ifdef ENABLE_IAPWS95
  IAPWS95App::registerApps();
#endif
}

void
CardinalApp::associateSyntaxInner(Syntax & syntax, ActionFactory & /* action_factory */)
{
  registerSyntax("VolumetricHeatSourceICAction", "Cardinal/ICs/VolumetricHeatSource");
  registerSyntax("BulkEnergyConservationICAction", "Cardinal/ICs/BulkEnergyConservation");

#ifdef ENABLE_OPENMC_COUPLING
  // Add the [Problem/Filters] block
  registerSyntaxTask("AddFilterAction", "Problem/Filters/*", "add_filters");
  registerMooseObjectTask("add_filters", Filter, false);
  addTaskDependency("add_filters", "init_displaced_problem");

  // Add the [Problem/Tallies] block
  registerSyntaxTask("AddTallyAction", "Problem/Tallies/*", "add_tallies");
  registerMooseObjectTask("add_tallies", Tally, false);
  addTaskDependency("add_tallies",
                    "add_filters"); // Make sure filters are constructed before tallies.
  // Can only add external auxvars after the tallies have been added.
  addTaskDependency("add_external_aux_variables", "add_tallies");
#endif

  registerTask("add_heat_source_ic", false /* is required */);
  addTaskDependency("add_heat_source_ic", "add_ic");

  registerTask("add_heat_source_postprocessor", false /* is required */);
  addTaskDependency("add_heat_source_postprocessor", "add_postprocessor");

  registerTask("add_bulk_fluid_temperature_ic", false /* is required */);
  addTaskDependency("add_bulk_fluid_temperature_ic", "add_bulk_fluid_temperature_user_object");

  registerTask("add_bulk_fluid_temperature_user_object", false /* is required */);
  addTaskDependency("add_bulk_fluid_temperature_user_object", "add_heat_source_ic");
}

std::string
CardinalApp::getInstallableInputs() const
{
  return CARDINAL_INSTALLABLE_DIRS;
}

/***************************************************************************************************
 *********************** Dynamic Library Entry Points - DO NOT MODIFY ******************************
 **************************************************************************************************/
extern "C" void
CardinalApp__registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  CardinalApp::registerAll(f, af, s);
}

extern "C" void
CardinalApp__registerApps()
{
  CardinalApp::registerApps();
}
