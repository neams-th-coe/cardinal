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

#include "MooseObjectAction.h"

/**
 * Initialize Nek application by calling nekrs::setup.
 * This action detects whether Nek should be initialized based on whether
 * NekRSProblem or NekRSStandaloneProblem are used as the Problem.
 */
class NekInitAction : public MooseObjectAction
{
public:
  static InputParameters validParams();

  NekInitAction(const InputParameters & parameters);

  virtual void act() override;

protected:
  /// whether a casename was provided in the input file
  const bool _casename_in_input_file;

  /**
   * Number of slices/slots to allocate in nrs->usrwrk to hold fields
   * for coupling (i.e. data going into NekRS, written by Cardinal), or
   * used for custom user actions, but not for coupling. By default, we just
   * allocate 7 slots (no inherent reason, just a fairly big amount). For
   * memory-limited cases, you can reduce this number to just the bare
   * minimum necessary for your use case.
   */
  const unsigned int & _n_usrwrk_slots;

  /// number of NekRS cases created
  static int _n_cases;
};
