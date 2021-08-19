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
  if (_type == "OpenMCCellAverageProblem" || _type == "OpenMCProblem")
  {
    int argc = 1;
    char openmc[] = "openmc";
    char * argv[1] = { openmc };

    openmc_init(argc, argv, &_communicator.get());
    // ensure that any mapped cells have their distribcell indices generated in OpenMC
    if (!openmc::settings::material_cell_offsets) {
      mooseWarning("Distributed properties for material cells are disabled "
                   "in the OpenMC settings. Enabling...");
      openmc::settings::material_cell_offsets = true;
      openmc::prepare_distribcell();
    }
  }
}
