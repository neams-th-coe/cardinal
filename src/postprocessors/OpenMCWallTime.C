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
  MooseEnum time_type("initialization_time total_simulation_time transport_time "
                      "inactive_batch_time active_batch_time fission_bank_time "
                      "tally_accumulation_time finalization_time total_elapsed_time",
                      "total_elapsed_time");
  params.addParam<MooseEnum>("time_type", time_type, "The time to report from OpenMC.");

  // We only get timing information after running OpenMC, so we force execution on TIMESTEP_END.
  params.set<ExecFlagEnum>("execute_on").addValidName("TIMESTEP_END");
  params.suppressParameter("execute_on");

  return params;
}

OpenMCWallTime::OpenMCWallTime(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    OpenMCBase(this, parameters),
    _accumulate_time(getParam<bool>("accumulate_time")),
    _openmc_time(getParam<MooseEnum>("time_type").getEnum<OpenMCTime>()),
    _walltime(0.0)
{ }

void
OpenMCWallTime::execute()
{
  if (!_accumulate_time)
    _walltime = 0.0;

  switch (_openmc_time)
  {
    case OpenMCTime::TotalInitTime:
      _walltime += openmc::simulation::time_initialize.elapsed();
      break;
    case OpenMCTime::TotalSimTime:
      _walltime +=
          openmc::simulation::time_inactive.elapsed() + openmc::simulation::time_active.elapsed();
      break;
    case OpenMCTime::TransportTime:
      _walltime += openmc::simulation::time_transport.elapsed();
      break;
    case OpenMCTime::InactiveBatchTime:
      _walltime += openmc::simulation::time_inactive.elapsed();
      break;
    case OpenMCTime::ActiveBatchTime:
      _walltime += openmc::simulation::time_active.elapsed();
      break;
    case OpenMCTime::FissionBankTime:
      _walltime += openmc::simulation::time_bank.elapsed();
      break;
    case OpenMCTime::TallyAccumTime:
      _walltime += openmc::simulation::time_tallies.elapsed();
      break;
    case OpenMCTime::FinalizationTime:
      _walltime += openmc::simulation::time_finalize.elapsed();
      break;
    case OpenMCTime::TotalTime:
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
