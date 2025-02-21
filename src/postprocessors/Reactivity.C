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

#include "Reactivity.h"

registerMooseObject("CardinalApp", Reactivity);

InputParameters
Reactivity::validParams()
{
  InputParameters params = KEigenvalue::validParams();
  params.addClassDescription("Calculate the reactivity based on the k-effective value.");
  return params;
}

Reactivity::Reactivity(const InputParameters & parameters) : KEigenvalue(parameters) {}

Real
Reactivity::getValue() const
{
  // Fetch the k-effective value using the parent class's getValue() method.
  Real k_effective = KEigenvalue::getValue();

  // Calculate and return the reactivity.
  return (k_effective - 1.0) / k_effective;
}
