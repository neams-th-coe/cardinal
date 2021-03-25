//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "NekPostprocessor.h"

class NekSidePostprocessor;

template <>
InputParameters validParams<NekSidePostprocessor>();

/**
 * Base class for performing sideset postprocessor operations within nekRS.
 * This class just checks that the boundary supplied by the user is a valid
 * ID within the nekRS mesh.
 */
class NekSidePostprocessor : public NekPostprocessor
{
public:
  static InputParameters validParams();

  NekSidePostprocessor(const InputParameters & parameters);

  virtual void initialize() override {}
  virtual void execute() override {}

protected:
  /// boundary IDs of the nekRS mesh over which to find the extreme value
  const std::vector<int> & _boundary;
};

