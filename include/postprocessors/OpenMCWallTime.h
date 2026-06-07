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

#pragma once

#include "GeneralPostprocessor.h"

#include "OpenMCBase.h"

/**
 * OpenMCWallTime is a post-processsor that returns the OpenMC wall time. This is one of
 * several options at present:
 * 1. Total time spent during initialization;
 * 2. Total time spent running a simulation;
 * 3. Total time running transport;
 * 4. Total time running inactive batches;
 * 5. Total time running active batches;
 * 6. Total time synchronizing the fission bank;
 * 7. Total time accumulating tallies;
 * 8. Total time spent in finalization;
 * 9. Total time elapsed.
 * The accumulated time (over all time steps / adaptivity steps / Picard iterations) is returned by
 * default to facilitate the computation of figures of merit. This behaviour can be adjusted
 * to return the wall time for a single simulation, if desired.
 */
class OpenMCWallTime : public GeneralPostprocessor, public OpenMCBase
{
public:
  static InputParameters validParams();

  OpenMCWallTime(const InputParameters & parameters);

  virtual void initialize() override {}
  virtual void execute() override;

  virtual Real getValue() const override;

  /// Declare the OpenMCTime enum. These will correspond 1 to 1 to the
  /// enums used in the input file.
  CreateMooseEnumClass(OpenMCTime,
    initialization_time = 0,
    total_simulation_time = 1,
    transport_time = 2,
    inactive_batch_time = 3,
    active_batch_time = 4,
    fission_bank_time = 5,
    tally_accumulation_time = 6,
    finalization_time = 7,
    total_elapsed_time = 8);

protected:
  /// Whether the simulation time should be accumulated or not.
  const bool & _accumulate_time;

  /// The type of time to report from OpenMC.
  const OpenMCTime _openmc_time;

  /// The accumulate or step walltime.
  Real _walltime;
};
