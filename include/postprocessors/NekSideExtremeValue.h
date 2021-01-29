//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "NekSidePostprocessor.h"
#include "CardinalEnums.h"

class NekSideExtremeValue;

template <>
InputParameters validParams<NekSideExtremeValue>();

/**
 * Compute the extreme value of a specified quantity over the boundaries
 * in the nekRS mesh.
 *
 * Note that this calculation is done directly on the mesh that nekRS solves on,
 * _not_ the mesh created for solution transfer in NekRSMesh.
 */
class NekSideExtremeValue : public NekSidePostprocessor
{
public:
  static InputParameters validParams();

  NekSideExtremeValue(const InputParameters & parameters);

  virtual Real getValue() override;

protected:
  /// field for which to find the extreme value
  const field::NekFieldEnum _field;

  /// type of extrema operation
  const operation::OperationEnum _type;
};

