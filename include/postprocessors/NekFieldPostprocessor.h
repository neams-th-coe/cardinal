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

#include "NekPostprocessor.h"
#include "CardinalEnums.h"

class NekFieldPostprocessor;

template <>
InputParameters validParams<NekFieldPostprocessor>();

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
