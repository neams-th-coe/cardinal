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

registerMooseAction("CardinalApp", OpenMCInitAction, "openmc_init");

InputParameters
OpenMCInitAction::validParams()
{
  InputParameters params = Action::validParams();
  params.addClassDescription("Initializes OpenMC.");
  params.addParam<std::string>("type", "Problem type");
  params.addParam<FileName>(
      "xml_directory", "./", "The directory in which to look for OpenMC XML files.");

  return params;
}

OpenMCInitAction::OpenMCInitAction(const InputParameters & parameters)
  : Action(parameters), _xml_directory(getParam<FileName>("xml_directory"))
{
}

void
OpenMCInitAction::act()
{
  if (_type != "OpenMCCellAverageProblem")
    return;


  // Suppress OpenMC output when the language server is active by
  // decreasing the verbosity to level 1 (the lowest).
  std::vector<std::string> argv_vec = {"openmc"};
  if (_app.isParamValid("language_server") && _app.getParam<bool>("language_server"))
  {
    argv_vec.push_back("-q");
    argv_vec.push_back("1");
  }
  // Add the parameter for the XML directory at the end.
  argv_vec.push_back(_xml_directory);

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
