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

class FissionTallyRelativeError;

template <>
InputParameters validParams<FissionTallyRelativeError>();

/**
 * Compute the max/min relative error of the kappa fission tally
 * added to extract the heat source from OpenMC.
 */
class FissionTallyRelativeError : public OpenMCPostprocessor
{
public:
  static InputParameters validParams();

  FissionTallyRelativeError(const InputParameters & parameters);

  virtual Real getValue() override;

protected:
  /// type of extrema operation
  const operation::OperationEnum _type;
};
