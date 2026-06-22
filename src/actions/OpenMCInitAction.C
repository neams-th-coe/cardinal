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
#include "AddMeshGeneratorAction.h"

registerMooseAction("CardinalApp", OpenMCInitAction, "openmc_init");

InputParameters
OpenMCInitAction::validParams()
{
  InputParameters params = Action::validParams();
  params.addClassDescription("Initializes OpenMC when an OpenMCCellAverageProblem or an "
                             "OpenMCMeshGenerator is present in the input.");
  return params;
}

OpenMCInitAction::OpenMCInitAction(const InputParameters & parameters) : Action(parameters) {}

void
OpenMCInitAction::act()
{
  // Check whether OpenMCCellAverageProblem is requested in the input file
  std::string xml_directory_problem;
  bool openmc_problem_requested = isOpenMCCellAverageProblemRequested(xml_directory_problem);

  // Check whether OpenMCMeshGenerator is requested in the input file
  std::string xml_directory_generator;
  bool openmc_mesh_generator_requested = isOpenMCMeshGeneratorRequested(xml_directory_generator);

  // if there is no need to initialize OpenMC, return
  if (!openmc_problem_requested && !openmc_mesh_generator_requested)
    return;

  // If both OpenMC problem and mesh generator are present, check xml_directory consistency
  if (openmc_problem_requested && openmc_mesh_generator_requested)
  {
    if (xml_directory_problem != xml_directory_generator)
    {
      mooseError("Inconsistent xml directories for OpenMC in the mesh generator and the problem "
                 "declarations.");
    }
  }

  // Select xml_directory
  std::string xml_directory =
      (openmc_problem_requested) ? xml_directory_problem : xml_directory_generator;

  // Initialize OpenMC
  initOpenMC(xml_directory);
}

bool
OpenMCInitAction::isOpenMCCellAverageProblemRequested(std::string & xml_directory) const
{
  // Retrieve all CreateProblemAction actions
  const auto & problem_actions = _awh.getActions<CreateProblemAction>();

  // Search for an OpenMCCellAverageProblem
  for (const auto * action : problem_actions)
  {
    if (action->getParam<std::string>("type") == "OpenMCCellAverageProblem")
    {
      xml_directory = action->getObjectParams().get<FileName>("xml_directory");
      return true;
    }
  }
  return false;
}

bool
OpenMCInitAction::isOpenMCMeshGeneratorRequested(std::string & xml_directory) const
{
  // Retrieve all AddMeshGeneratorAction actions
  const auto & mesh_gen_actions = _awh.getActions<AddMeshGeneratorAction>();

  // Search for OpenMCMeshGenerators and verify that xml_directory parameters are consistent
  bool found = false;
  for (const auto * action : mesh_gen_actions)
  {
    if (action->getMooseObjectType() == "OpenMCMeshGenerator")
    {
      std::string xml_directory_temp = action->getObjectParams().get<FileName>("xml_directory");
      if (found)
      {
        if (xml_directory_temp != xml_directory)
        {
          mooseError(
              "Inconsistent xml directories for OpenMC in the declared OpenMCMeshGenerators");
        }
      }
      else
      {
        xml_directory = xml_directory_temp;
        found = true;
      }
    }
  }
  if (found)
  {
    return true;
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
