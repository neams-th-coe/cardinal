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
}

Real
PecletNumber::getValue()
{
  return ReynoldsNumber::getValue() * nekrs::Pr();
}
