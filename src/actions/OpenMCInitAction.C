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

#ifdef ENABLE_OPENMC_COUPLING

#include "OpenMCInitAction.h"
#include "openmc/capi.h"
#include "openmc/settings.h"
#include "openmc/geometry_aux.h"

registerMooseAction("CardinalApp", OpenMCInitAction, "openmc_init");

InputParameters
OpenMCInitAction::validParams()
{
  InputParameters params = MooseObjectAction::validParams();
  params.addParam<std::string>("type", "Problem type");
  return params;
}

OpenMCInitAction::OpenMCInitAction(const InputParameters & parameters)
  : MooseObjectAction(parameters)
{
}

void
OpenMCInitAction::act()
{
  TIME_SECTION("initOpenMC", 1, "Initializing OpenMC", false);

  if (_type == "OpenMCCellAverageProblem")
  {
    int argc = 1;
    char openmc[] = "openmc";
    char * argv[1] = {openmc};

    openmc_init(argc, argv, &_communicator.get());

    // ensure that any mapped cells have their distribcell indices generated in OpenMC
    if (!openmc::settings::material_cell_offsets)
    {
      mooseWarning("Distributed properties for material cells are disabled "
                   "in the OpenMC settings. Enabling...");
      openmc::settings::material_cell_offsets = true;
      openmc::prepare_distribcell();
    }

    // ensure that unsupported run modes are not used
    auto run_mode = openmc::settings::run_mode;
    switch (run_mode)
    {
      case openmc::RunMode::EIGENVALUE:
      case openmc::RunMode::FIXED_SOURCE:
        break;
      case openmc::RunMode::PLOTTING:
        mooseError("Running OpenMC in plotting mode is not supported through Cardinal!\n"
          "Please just run using the OpenMC executable, like 'openmc --plot'");
      case openmc::RunMode::PARTICLE:
        mooseError("Running OpenMC in particle restart mode is not supported through Cardinal!\n"
         "Please just run using the OpenMC executable, like 'openmc --restart <binary_file>'");
      case openmc::RunMode::VOLUME:
        mooseError("Running OpenMC in volume calculation mode is not supported through Cardinal!\n"
          "Please just run using the OpenMC executable, like 'openmc --volume'");
      default:
        mooseError("Unhandled openmc::RunMode enum in OpenMCInitAction!");
    }
  }
}

#endif
