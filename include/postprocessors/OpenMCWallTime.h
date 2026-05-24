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

protected:
  /// Whether the simulation time should be accumulated or not.
  const bool & _accumulate_time;

  /// The type of time to report from OpenMC.
  const enum class OpenMCTime
  {
    TotalInitTime = 0,
    TotalSimTime = 1,
    TransportTime = 2,
    InactiveBatchTime = 3,
    ActiveBatchTime = 4,
    FissionBankTime = 5,
    TallyAccumTime = 6,
    FinalizationTime = 7,
    TotalTime = 8
  } _openmc_time;

  Real _walltime;
};
