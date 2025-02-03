#pragma once

#include "InputParameters.h"
#include "MooseObject.h"
#include "CardinalEnums.h"

class NekFieldInterface
{
public:
  static InputParameters validParams();

  NekFieldInterface(const MooseObject * moose_object, const InputParameters & parameters);

protected:
  /// NekRS solution field
  const field::NekFieldEnum _field;

  /// Direction in which to evaluate velocity, when field = velocity_component
  Point _velocity_direction;
};
