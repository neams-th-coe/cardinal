#pragma once

#include "MooseObjectAction.h"

/**
 * Initialize OpenMC application by calling openmc_init.
 * This action detects whether OpenMC should be initialized based on whether
 * OpenMCCellAverageProblem or OpenMCProblem are used as the Problem.
 */
class OpenMCInitAction : public MooseObjectAction
{
public:
  static InputParameters validParams();

  OpenMCInitAction(const InputParameters & parameters);

  virtual void act() override;
};
