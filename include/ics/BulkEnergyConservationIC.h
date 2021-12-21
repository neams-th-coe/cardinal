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

#include "InitialCondition.h"
#include "FunctionLayeredIntegral.h"

class BulkEnergyConservationIC;
class InputParameters;

template <typename T>
InputParameters validParams();

/**
 * Applies a temperature initial condition based on bulk energy conservation
 * in a fluid without any losses
 */
class BulkEnergyConservationIC : public InitialCondition
{
public:
  static InputParameters validParams();

  BulkEnergyConservationIC(const InputParameters & parameters);

  virtual void initialSetup() override;

protected:
  virtual Real value(const Point & p) override;

  /// Cumulative integral of the heat source in the direction of flow
  const FunctionLayeredIntegral & _layered_integral;

  /// Fluid mass flowrate
  const Real & _mdot;

  /// Fluid isobaric specific heat capacity
  const Real & _cp;

  /// Fluid inlet temperature
  const Real & _inlet_T;

  /// Name of postprocessor providing the integral of the heat source
  const PostprocessorName & _pp_name;

  /// Value of the postprocessor providing the integral of the heat source
  const PostprocessorValue & _integral;

  /// Total magnitude of the heat source
  const Real & _magnitude;
};
