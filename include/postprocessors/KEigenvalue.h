//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "OpenMCPostprocessor.h"
#include "CardinalEnums.h"

class KEigenvalue;

template <>
InputParameters validParams<KEigenvalue>();

/**
 * Get the k-effective eigenvalue computed by OpenMC.
 */
class KEigenvalue : public OpenMCPostprocessor
{
public:
  static InputParameters validParams();

  KEigenvalue(const InputParameters & parameters);

  virtual Real getValue() override;

protected:
  /**
   * Type of k-effective value to report. Options:
   * collision, absorption, tracklength, and combined (default).
   *
   * The combined k-effective estimate is a minimum variance estimate
   * of k-effective based on a linear combination of the collision, absorption,
   * and tracklength estimates.
   */
  const eigenvalue::EigenvalueEnum _type;
};
