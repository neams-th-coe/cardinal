#pragma once

#include "MooseObjectAction.h"

/**
 * Initialize Nek application by calling nekrs::setup.
 * This action detects whether Nek should be initialized based on whether
 * NekRSProblem or NekProblem are used as the Problem.
 */
class NekInitAction : public MooseObjectAction
{
public:
  static InputParameters validParams();

  NekInitAction(const InputParameters & parameters);

  virtual void act() override;
};
