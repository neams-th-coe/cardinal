#include "OpenMCInitAction.h"
#include "openmc/capi.h"

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
  }
}
