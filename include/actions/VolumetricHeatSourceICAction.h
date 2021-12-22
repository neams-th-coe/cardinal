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

class VolumetricHeatSourceICAction;

/**
 * Action that automatically sets up a volumetric heat source initial
 * condition as a wrapping of a FunctionElementIntegral and an
 * IntegralPreservingFunctionIC.
 */
class VolumetricHeatSourceICAction : public CardinalAction
{
public:
  VolumetricHeatSourceICAction(const InputParameters & parameters);

  static InputParameters validParams();

  virtual void act();

protected:
  /// Variable name to apply the initial condition to
  const VariableName & _variable;

  /// Functional form for the heat source
  const FunctionName & _function;

  /// Total magnitude of the heat source upon integration
  const Real & _magnitude;
};
