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

#include "NekHeatFluxIntegral.h"

registerMooseObject("CardinalApp", NekHeatFluxIntegral);

InputParameters
NekHeatFluxIntegral::validParams()
{
  InputParameters params = NekSidePostprocessor::validParams();
  params.addClassDescription("Compute heat flux over a boundary in the NekRS mesh");
  return params;
}

NekHeatFluxIntegral::NekHeatFluxIntegral(const InputParameters & parameters)
  : NekSidePostprocessor(parameters)
{
  // this postprocessor computes the gradient of temperature, so it requires
  // the temperature field to exist
  if (!nekrs::hasTemperatureVariable())
    mooseError("This postprocessor can only be used with NekRS problems that have a temperature "
               "variable!");
}

Real
NekHeatFluxIntegral::getValue() const
{
  return nekrs::heatFluxIntegral(_boundary, _pp_mesh);
}

#endif
