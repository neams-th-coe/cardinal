#pragma once

#include "NekPostprocessor.h"
#include "CardinalEnums.h"

/**
 * Base class for all NekRS postprocessors that operate on fields
 * on boundaries of the NekRS domain, such as for averaging a field
 * variable on a sideset.
 */
class NekSideFieldPostprocessor : public NekSidePostprocessor
{
public:
  static InputParameters validParams();

  NekSideFieldPostprocessor(const InputParameters & parameters);

protected:
  /// integrand of the surface integral
  const field::NekFieldEnum _field;

  /// Direction in which to evaluate velocity, when field = velocity_component
  Point _velocity_direction;
};

