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

#include "OpenMCWallTime.h"

#include "openmc/timer.h"

registerMooseObject("CardinalApp", OpenMCWallTime);

InputParameters
OpenMCWallTime::validParams()
{
  auto params = GeneralPostprocessor::validParams();
  params += OpenMCBase::validParams();
  params.addClassDescription("A post-processor which reports OpenMC walltime.");
  params.addParam<bool>("accumulate_time",
                        true,
                        "Whether the simulation time should be accumulated over all simulation "
                        "steps (selected by default) or not.");
  params.addParam<MooseEnum>("time_type",
                             MooseEnum(getOpenMCTimeOptions(), "total_elapsed_time"),
                             "The time to report from OpenMC.");

  // We only get timing information after running OpenMC, so we force execution on TIMESTEP_END.
  params.set<ExecFlagEnum>("execute_on").clearSetValues();
  params.set<ExecFlagEnum>("execute_on") = "TIMESTEP_END";
  params.suppressParameter<ExecFlagEnum>("execute_on");

  return params;
}

OpenMCWallTime::OpenMCWallTime(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    OpenMCBase(this, parameters),
    _accumulate_time(getParam<bool>("accumulate_time")),
    _openmc_time(getParam<MooseEnum>("time_type").getEnum<OpenMCTime>()),
    _walltime(0.0)
{
}

void
OpenMCWallTime::execute()
{
  if (!_accumulate_time)
    _walltime = 0.0;

  switch (_openmc_time)
  {
    case OpenMCTime::initialization_time:
      _walltime += openmc::simulation::time_initialize.elapsed();
      break;
    case OpenMCTime::total_simulation_time:
      _walltime +=
          openmc::simulation::time_inactive.elapsed() + openmc::simulation::time_active.elapsed();
      break;
    case OpenMCTime::transport_time:
      _walltime += openmc::simulation::time_transport.elapsed();
      break;
    case OpenMCTime::inactive_batch_time:
      _walltime += openmc::simulation::time_inactive.elapsed();
      break;
    case OpenMCTime::active_batch_time:
      _walltime += openmc::simulation::time_active.elapsed();
      break;
    case OpenMCTime::fission_bank_time:
      _walltime += openmc::simulation::time_bank.elapsed();
      break;
    case OpenMCTime::tally_accumulation_time:
      _walltime += openmc::simulation::time_tallies.elapsed();
      break;
    case OpenMCTime::finalization_time:
      _walltime += openmc::simulation::time_finalize.elapsed();
      break;
    case OpenMCTime::total_elapsed_time:
      _walltime += openmc::simulation::time_total.elapsed();
      break;
    default:
      mooseError("Unhandled OpenMCTime enum!");
  }
}

Real
OpenMCWallTime::getValue() const
{
  return _walltime;
}

#endif
