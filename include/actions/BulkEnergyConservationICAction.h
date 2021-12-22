/********************************************************************/
/*                  SOFTWARE COPYRIGHT NOTIFICATION                 */
/*                             Cardinal                             */
/*                                                                  */
/*                  (c) 2021 UChicago Argonne, LLC                  */
/*                        ALL RIGHTS RESERVED                       */
/*                                                                  */
/*                 Prepared by UChicago Argonne, LLC                */
/*               Under Contract No. DE-AC02-06CH11357               */
/*                With the U. S. Department of Energy               */
/*                                                                  */
/*             Prepared by Battelle Energy Alliance, LLC            */
/*               Under Contract No. DE-AC07-05ID14517               */
/*                With the U. S. Department of Energy               */
/*                                                                  */
/*                 See LICENSE for full restrictions                */
/********************************************************************/

#pragma once

#include "CardinalAction.h"

class BulkEnergyConservationICAction;

/**
 * Action that automatically applies a fluid temperature initial condition
 * based on bulk energy conservation.
 */
class BulkEnergyConservationICAction : public CardinalAction
{
public:
  BulkEnergyConservationICAction(const InputParameters & parameters);

  static InputParameters validParams();

  virtual void act();

protected:
  /// Variable(s) to which the initial condition should be applied
  const std::vector<VariableName> & _variable;

  /// Fluid mass flowrate
  const Real & _mdot;

  /// Fluid isobaric specific heat capacity
  const Real & _cp;

  /// Fluid inlet temperature
  const Real & _inlet_T;

  /// Number of layers to integrate the heat source in
  const unsigned int & _num_layers;

  /// Direction the fluid flows
  const MooseEnum _direction;

  /// Whether the flow is in the positive or negative direction
  const bool & _positive_flow_direction;

  /// Whether a minimum direction was specified
  const bool _has_direction_min;

  /// Whether a maximum direction was specified
  const bool _has_direction_max;

  /// Minimum coordinate of the layers
  const Real * _direction_min;

  /// Maximum coordinate of the layers
  const Real * _direction_max;
};
