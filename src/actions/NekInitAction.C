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

#ifdef ENABLE_NEK_COUPLING

#include "NekInitAction.h"
#include "NekInterface.h"
#include "nekrs.hpp"

#include "MooseApp.h"
#include "CommandLine.h"
#include <chrono>

registerMooseAction("CardinalApp", NekInitAction, "nek_init");

int NekInitAction::_n_cases = 0;

InputParameters
NekInitAction::validParams()
{
  InputParameters params = MooseObjectAction::validParams();
  params.addParam<std::string>("type", "Problem type");
  params.addParam<std::string>(
      "casename",
      "Case name for the NekRS input files; "
      "this is <case> in <case>.par, <case>.udf, <case>.oudf, and <case>.re2.");

  params.addParam<unsigned int>("n_usrwrk_slots", 7,
    "Number of slots to allocate in nrs->usrwrk to hold fields either related to coupling "
    "(which will be populated by Cardinal), or other custom usages, such as a distance-to-wall calculation");

  return params;
}

NekInitAction::NekInitAction(const InputParameters & parameters)
  : MooseObjectAction(parameters),
    _specified_scratch(parameters.isParamSetByUser("n_usrwrk_slots")),
    _n_usrwrk_slots(getParam<unsigned int>("n_usrwrk_slots"))
{
}

void
NekInitAction::act()
{
  bool is_nek_problem = _type == "NekRSProblem" || _type == "NekRSStandaloneProblem" ||
                        _type == "NekRSSeparateDomainProblem";

  if (!is_nek_problem)
    return;

  const auto timeStart = std::chrono::high_resolution_clock::now();

  // NekRS does some checks in main(); because we don't call that function
  // directly, repeat those checks here
  if (!getenv("NEKRS_HOME"))
    mooseError("Cannot find environment variable NEKRS_HOME!");

  std::string bin(getenv("NEKRS_HOME"));
  bin += "/bin/nekrs";
  const char * ptr = realpath(bin.c_str(), NULL);
  if (!ptr)
    mooseError("Cannot find '", bin, "'! Did you set NEKRS_HOME to the correct location?");

  std::string setup_file = getParam<std::string>("casename");

  // If the casename is a directory path (i.e. not just a file name), then standalone
  // NekRS will cd into that directory so that the casename really is just the case file name,
  // i.e. with the path subtracted out. We will replicate this behavior here.
  std::size_t last_slash = setup_file.rfind('/') + 1;
  std::string casepath = setup_file.substr(0, last_slash);
  std::string casename = setup_file.substr(last_slash, setup_file.length() - last_slash);
  if (casepath.length() > 0)
  {
    int fail = chdir(casepath.c_str());
    if (fail)
      mooseError("Failed to find '", casepath.c_str(), "'! Did you set the 'casename' correctly?");
  }

  // we need to set the default values here because it seems that the default values
  // that can be set via addCommandLineParam in CardinalApp aren't propagated through the 'search'
  // function
  int size_target = 0;
  int ci_mode = 0;
  std::string backend = "";
  std::string device_id = "";

  std::shared_ptr<CommandLine> cl = _app.commandLine();
  cl->search("nekrs_buildonly", size_target);
  cl->search("nekrs_cimode", ci_mode);
  cl->search("nekrs_backend", backend);
  cl->search("nekrs_device_id", device_id);

  int build_only = size_target > 0 ? 1 : 0;
  nekrs::buildOnly(build_only);

  MPI_Comm comm = *static_cast<const MPI_Comm *>(&_communicator.get());

  // If this MPI communicator has already created one case, then we cannot also create a
  // second NekRS case. For instance, if you have 4 Nek sub-apps, but only 3 processes, then
  // NekRS doesn't like trying to set up a case with a communicator, then immediately try
  // to set up another case with the same communicator. If you un-comment this error message,
  // you'll get an error when reading the mesh file that is basically missing a "/".
  // This error is probably due to NekRS relying a lot on static variables.
  if (_n_cases > 0)
  {
    int size;
    MPI_Comm_size(comm, &size);
    mooseError("NekRS does not currently support setting up multiple cases with the same "
               "MPI communicator.\nThat is, you need at least one MPI process in a master "
               "application per Nek sub-application.\n\n"
               "The MPI communicator has " +
               std::to_string(size) +
               " ranks and is trying to "
               "construct " +
               std::to_string(_n_cases + 1) + "+ cases.\n\n"
               "If you are running a Stochastic Tools simulation in either 'normal' or 'batch-reset'\n"
               "mode, you will need at least 'min_procs_per_app' * 'num_rows' MPI ranks. OR, we\n"
               "recommend using the 'batch-restore' mode, which does not have any such limitations.");
  }

  nekrs::setup(comm /* global communicator, like for Nek-Nek : NOT SUPPORTED, so we use same comm */,
               comm /* local communicator */,
               build_only,
               size_target,
               ci_mode,
               casename,
               backend,
               device_id,
               1 /* n sessions */,
               0 /* session ID */,
               0 /* debug mode */);

  _n_cases++;

  // copy-pasta from NekRS's main()
  double elapsedTime = 0;
  const auto timeStop = std::chrono::high_resolution_clock::now();
  elapsedTime += std::chrono::duration<double, std::milli>(timeStop - timeStart).count() / 1e3;
  MPI_Allreduce(MPI_IN_PLACE, &elapsedTime, 1, MPI_DOUBLE, MPI_MAX, comm);
  nekrs::updateTimer("setup", elapsedTime);
  nekrs::setNekSetupTime(elapsedTime);

  _console << "initialization took " << elapsedTime << " s" << std::endl;

  // setup actions only needed if coupling with MOOSE
  if (_type == "NekRSProblem")
  {
    // First check we should do is that a temperature variable exists, or else many
    // of our indexes into `nrs->cds` would give seg faults
    if (!nekrs::hasTemperatureVariable())
      mooseError(
          "To properly transfer temperature and heat flux between nekRS and MOOSE, "
          "your nekRS model must include a solution for temperature.\n\nDid you forget the "
          "TEMPERATURE block in '" + setup_file + ".par'?\nNote: you can set 'solver = none' in '" +
          setup_file + ".par' if you don't want to solve for temperature.");
  }

  // Initialize default dimensional scales assuming a dimensional run is performed;
  // these are overriden if using a non-dimensional solve
  nekrs::initializeDimensionalScales(1.0 /* U_ref */,
                                     0.0 /* T_ref */,
                                     1.0 /* dT_ref */,
                                     1.0 /* L_ref */,
                                     1.0 /* rho_ref */,
                                     1.0 /* Cp_ref */);

  bool always_allocate = _type == "NekRSProblem" || _type == "NekRSSeparateDomainProblem";
  bool special_allocate = _type == "NekRSStandaloneProblem" && _specified_scratch;
  if (always_allocate || special_allocate)
  {
    if (!nekrs::scratchAvailable())
      mooseError(
          "The nrs_t.usrwrk and nrs_t.o_usrwrk arrays are automatically allocated by Cardinal,\n"
          "but you have tried allocating them separately inside your case files. Please remove the\n"
          "manual allocation of the space in your user files, and be sure to only write such that\n"
          "the space reserved for coupling data is untouched.");

    // Initialize scratch space in NekRS to write data incoming data from MOOSE
    nekrs::initializeScratch(_n_usrwrk_slots);
  }
}

#endif
