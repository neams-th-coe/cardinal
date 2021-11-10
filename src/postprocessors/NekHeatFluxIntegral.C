#include "NekHeatFluxIntegral.h"
#include "CardinalEnums.h"

registerMooseObject("CardinalApp", NekHeatFluxIntegral);

defineLegacyParams(NekHeatFluxIntegral);

InputParameters
NekHeatFluxIntegral::validParams()
{
  InputParameters params = NekSidePostprocessor::validParams();
  params.addClassDescription("Compute heat flux over a boundary in the NekRS mesh");
  return params;
}

NekHeatFluxIntegral::NekHeatFluxIntegral(const InputParameters & parameters) :
  NekSidePostprocessor(parameters)
{
  // this postprocessor computes the gradient of temperature, so it requires
  // the temperature field to exist
  checkValidField(field::temperature);
}

Real
NekHeatFluxIntegral::getValue()
{
  return nekrs::heatFluxIntegral(_boundary);
}
