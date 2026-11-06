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
  params.addClassDescription("Heat flux over a boundary in the NekRS mesh");
  return params;
}

NekHeatFluxIntegral::NekHeatFluxIntegral(const InputParameters & parameters)
  : NekSidePostprocessor(parameters)
{
  mooseDeprecated("This object is deprecated; replace it with NekDiffusiveFluxIntegral.");
}

Real
NekHeatFluxIntegral::getValue() const
{
  return nekrs::diffusiveFluxIntegral(_boundary, 0, _pp_mesh);
}

#endif
