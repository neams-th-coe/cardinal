#include "PecletNumber.h"
#include "NekInterface.h"

registerMooseObject("CardinalApp", PecletNumber);

defineLegacyParams(PecletNumber);

InputParameters
PecletNumber::validParams()
{
  InputParameters params = ReynoldsNumber::validParams();
  params.addClassDescription("Compute the Peclet number characteristic of the NekRS solution");
  return params;
}

PecletNumber::PecletNumber(const InputParameters & parameters) :
  ReynoldsNumber(parameters)
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
PecletNumber::getValue()
{
  return ReynoldsNumber::getValue() * nekrs::Pr();
}
