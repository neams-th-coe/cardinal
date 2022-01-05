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

#include "OpenMCProblemBase.h"

#include "openmc/capi.h"
#include "openmc/error.h"
#include "openmc/settings.h"

InputParameters
OpenMCProblemBase::validParams()
{
  InputParameters params = ExternalProblem::validParams();
  params.addRequiredRangeCheckedParam<Real>("power", "power >= 0.0",
    "Power (Watts) to normalize the OpenMC tallies");
  params.addParam<bool>("verbose", false, "Whether to print diagnostic information");

  // interfaces to directly set some OpenMC parameters
  params.addRangeCheckedParam<unsigned int>("openmc_verbosity",
    "openmc_verbosity >= 1 & openmc_verbosity <= 10",
    "OpenMC verbosity level; this overrides the setting in the XML files");
  params.addRangeCheckedParam<unsigned int>("inactive_batches", "inactive_batches > 0",
    "Number of inactive batches to run in OpenMC; this overrides the setting in the XML files.");
  params.addRangeCheckedParam<int64_t>("particles", "particles > 0 ",
    "Number of particles to run in each OpenMC batch; this overrides the setting in the XML files.");
  params.addRangeCheckedParam<unsigned int>("batches", "batches > 0",
    "Number of batches to run in OpenMC; this overrides the setting in the XML files.");
  return params;
}

OpenMCProblemBase::OpenMCProblemBase(const InputParameters &params) :
  ExternalProblem(params),
  _power(getParam<Real>("power")),
  _verbose(getParam<bool>("verbose"))
{
  if (isParamValid("openmc_verbosity"))
    openmc::settings::verbosity = getParam<unsigned int>("openmc_verbosity");

  if (isParamValid("inactive_batches"))
    openmc::settings::n_inactive = getParam<unsigned int>("inactive_batches");

  if (isParamValid("particles"))
    openmc::settings::n_particles = getParam<int64_t>("particles");

  if (isParamValid("batches"))
  {
    auto xml_n_batches = openmc::settings::n_batches;

    int err = openmc_set_n_batches(getParam<unsigned int>("batches"),
      true /* set the max batches */,
      true /* add the last batch for statepoint writing */);

    if (err)
      mooseError("In attempting to set the number of batches, OpenMC reported:\n\n" +
        std::string(openmc_err_msg));

    // if we set the batches from Cardinal, remove whatever statepoint file was
    // created for the #batches set in the XML files; this is just to reduce the
    // number of statepoint files by removing an unnecessary point
    openmc::settings::statepoint_batch.erase(xml_n_batches);
  }
}

OpenMCProblemBase::~OpenMCProblemBase()
{
}
