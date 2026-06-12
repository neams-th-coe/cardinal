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
#include "CreateProblemAction.h"

registerMooseAction("CardinalApp", OpenMCInitAction, "openmc_init");

InputParameters
OpenMCInitAction::validParams()
{
  InputParameters params = Action::validParams();
  params.addClassDescription(
      "Initializes OpenMC when an OpenMCCellAverageProblem is present in the input.");
  return params;
}

OpenMCInitAction::OpenMCInitAction(const InputParameters & parameters) : Action(parameters) {}

void
OpenMCInitAction::act()
{
  // Check if OpenMCCellAverageProblem is present and return xml_directory if it is the case
  std::string xml_directory;

  // Leave if no OpenMCCellAverageProblem requested in the input file
  if (!isOpenMCCellAverageProblemRequested(xml_directory))
    return;

  // Initialize OpenMC
  initOpenMC(xml_directory);
}

bool
OpenMCInitAction::isOpenMCCellAverageProblemRequested(std::string & xml_directory) const
{
  // Retrieve all create problem actions
  const auto & problem_actions = _awh.getActions<CreateProblemAction>();

  for (const auto * action : problem_actions)
  {
    // If an action has the OpenMCCellAverageProblem type
    if (action->getParam<std::string>("type") == "OpenMCCellAverageProblem")
    {
      xml_directory = action->getObjectParams().get<FileName>("xml_directory");
      return true;
    }
  }
  return false;
}

void
OpenMCInitAction::initOpenMC(const std::string & xml_directory)
{
  // Suppress OpenMC output when the language server is active by
  // decreasing the verbosity to level 1 (the lowest).
  std::vector<std::string> argv_vec = {"openmc"};
  if (_app.isParamValid("language_server") && _app.getParam<bool>("language_server"))
  {
    argv_vec.push_back("-q");
    argv_vec.push_back("1");
  }

  // Add xml_directory
  argv_vec.push_back(xml_directory);

  std::vector<char *> argv;
  for (const auto & arg : argv_vec)
  {
    argv.push_back(const_cast<char *>(arg.data()));
  }

  // Add terminating nullptr
  argv.push_back(nullptr);

  // Initialize OpenMC
  openmc_init(argv.size() - 1, argv.data(), &_communicator.get());
}

#endif
