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

#ifdef ENABLE_NEK_COUPLING

#include "PecletNumber.h"
#include "NekInterface.h"

registerMooseObject("CardinalApp", PecletNumber);

InputParameters
PecletNumber::validParams()
{
  InputParameters params = ReynoldsNumber::validParams();
  params.addClassDescription("Compute the Peclet number characteristic of the NekRS solution");
  return params;
}

PecletNumber::PecletNumber(const InputParameters & parameters) : ReynoldsNumber(parameters)
{
  // rhoCp and k don't get initialized if the temperature solve is turned off,
  // even if the temperature variable exists. Because the user could technically still
  // be setting these parameters manually in the .udf file (which we do ourselves
  // for testing), just throw an error if temperature doesn't exist at all
  if (!nekrs::hasTemperatureVariable())
    mooseError("This postprocessor cannot be used in NekRS problems "
               "without a temperature variable!");
}

Real
PecletNumber::getValue() const
{
  return ReynoldsNumber::getValue() * nekrs::Pr();
}

#endif
