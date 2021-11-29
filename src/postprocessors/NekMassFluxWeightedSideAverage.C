#include "NekMassFluxWeightedSideAverage.h"

registerMooseObject("CardinalApp", NekMassFluxWeightedSideAverage);

InputParameters
NekMassFluxWeightedSideAverage::validParams()
{
  InputParameters params = NekMassFluxWeightedSideIntegral::validParams();
  params.addClassDescription("Compute mass flux weighted average of a field over a boundary in the NekRS mesh");
  return params;
}

NekMassFluxWeightedSideAverage::NekMassFluxWeightedSideAverage(const InputParameters & parameters) :
  NekMassFluxWeightedSideIntegral(parameters)
{
}

Real
NekMassFluxWeightedSideAverage::getValue()
{
  return NekMassFluxWeightedSideIntegral::getValue() / nekrs::massFlowrate(_boundary);
}
