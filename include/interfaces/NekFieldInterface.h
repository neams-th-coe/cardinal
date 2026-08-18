#pragma once

#include "InputParameters.h"
#include "MooseObject.h"
#include "CardinalEnums.h"
#include "FunctionInterface.h"

class NekFieldInterface : public FunctionInterface
{
public:
  static InputParameters validParams();

  NekFieldInterface(const MooseObject * moose_object,
                    const InputParameters & parameters,
                    const bool allow_normal = false);

  /**
   * Get the field
   * @return field
   */
  const field::NekFieldEnum & field() const { return _field; }

protected:
  /// NekRS solution field
  const field::NekFieldEnum _field;

  /// Function to shift the field by, if provided by the 'function' parameter
  const Function * _function;

  /**
   * Direction in which to evaluate velocity, if using 'field = velocity_component'.
   * Options: user (then provide a general vector direction with the 'velocity_direction' parameter
   *          normal (normal to an interface, context dependent on daughter class)
   */
  component::BinnedVelocityComponentEnum _velocity_component;

  /// Direction in which to evaluate velocity, when field = velocity_component
  Point _velocity_direction;
};
