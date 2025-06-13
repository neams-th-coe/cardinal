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

#include "MooseApp.h"
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

  params.addParam<unsigned int>(
      "n_usrwrk_slots",
      7,
      "Number of slots to allocate in nrs->usrwrk to hold fields either related to coupling "
      "(which will be populated by Cardinal), or other custom usages, such as a distance-to-wall "
      "calculation");

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
  if (_type != "NekRSProblem")
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

  const int size_target =
      _app.isParamValid("nekrs_build_only") ? _app.getParam<int>("nekrs_build_only") : 0;
  const int ci_mode = _app.isParamValid("nekrs_cimode") ? _app.getParam<int>("nekrs_cimode") : 0;
  const std::string backend =
      _app.isParamValid("nekrs_backend") ? _app.getParam<std::string>("nekrs_backend") : "";
  const std::string device_id =
      _app.isParamValid("nekrs_device_id") ? _app.getParam<std::string>("nekrs_device_id") : "";

  const int build_only = size_target > 0 ? 1 : 0;
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
    mooseError(
        "NekRS does not currently support setting up multiple cases with the same "
        "MPI communicator.\nThat is, you need at least one MPI process in a master "
        "application per Nek sub-application.\n\n"
        "The MPI communicator has " +
        std::to_string(size) +
        " ranks and is trying to "
        "construct " +
        std::to_string(_n_cases + 1) +
        "+ cases.\n\n"
        "If you are running a Stochastic Tools simulation in either 'normal' or 'batch-reset'\n"
        "mode, you will need at least 'min_procs_per_app' * 'num_rows' MPI ranks. OR, we\n"
        "recommend using the 'batch-restore' mode, which does not have any such limitations.");
  }

  auto par = readPar(casename, comm);

  nekrs::setup(
      comm /* global communicator, like for Nek-Nek : NOT SUPPORTED, so we use same comm */,
      comm /* local communicator */,
      build_only,
      size_target,
      ci_mode,
      par,
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

  if (!nekrs::scratchAvailable())
    mooseError(
        "The nrs_t.usrwrk and nrs_t.o_usrwrk arrays are automatically allocated by Cardinal, "
        "but you have tried allocating them separately inside your case files. Please remove the "
        "manual allocation of the space in your user files, and be sure to only write such that"
        "the space reserved for coupling data is untouched.");

  // Initialize scratch space in NekRS to write data incoming data from MOOSE
  nekrs::initializeScratch(_n_usrwrk_slots);
}

inipp::Ini *
NekInitAction::readPar(const std::string & _setupFile, MPI_Comm comm)
{
  auto par = new inipp::Ini();

  int rank;
  MPI_Comm_rank(comm, &rank);

  const auto setupFile = _setupFile + ".par";

  int err = 0;
  if (rank == 0)
  {
    if (!std::filesystem::exists(setupFile))
    {
      std::cerr << "Cannot find setup file " << setupFile << std::endl;
      err++;
    }
  }
  MPI_Allreduce(MPI_IN_PLACE, &err, 1, MPI_INT, MPI_MAX, comm);
  if (err)
  {
    MPI_Abort(comm, EXIT_FAILURE);
  }

  char * rbuf;
  long fsize;

  if (rank == 0)
  {
    FILE * f = fopen(setupFile.c_str(), "rb");
    fseek(f, 0, SEEK_END);
    fsize = ftell(f);
    fseek(f, 0, SEEK_SET);
    rbuf = new char[fsize];
    auto s = fread(rbuf, 1, fsize, f);
    fclose(f);
  }
  MPI_Bcast(&fsize, sizeof(fsize), MPI_BYTE, 0, comm);

  if (rank != 0)
    rbuf = new char[fsize];
  MPI_Bcast(rbuf, fsize, MPI_CHAR, 0, comm);

  std::stringstream is;
  is.write(rbuf, fsize);

  par->parse(is);
  par->interpolate();

  return par;
}

#endif
