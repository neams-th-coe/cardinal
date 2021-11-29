#pragma once

#include "NekPostprocessor.h"
#include "CardinalEnums.h"

/**
 * Base class for NekRS postprocessors that operate on fields,
 * such as for taking averages of a field variable.
 */
class NekFieldPostprocessor : public NekPostprocessor
{
public:
  static InputParameters validParams();

  NekFieldPostprocessor(const InputParameters & parameters);

protected:
  /// integrand of the surface integral
  const field::NekFieldEnum _field;

  /// Direction in which to evaluate velocity, when field = velocity_component
  Point _velocity_direction;
};
