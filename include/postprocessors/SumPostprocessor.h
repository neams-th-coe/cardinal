//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralPostprocessor.h"

class SumPostprocessor;

template <>
InputParameters validParams<SumPostprocessor>();

/**
 * Computes the sum of two postprocessors
 *
 * result = value1 + value2
 */
class SumPostprocessor : public GeneralPostprocessor
{
public:
  static InputParameters validParams();

  SumPostprocessor(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual PostprocessorValue getValue() override;

protected:
  const PostprocessorValue & _value1;
  const PostprocessorValue & _value2;
};

