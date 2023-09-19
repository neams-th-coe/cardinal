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

#include "NekMassFluxWeightedSideAverage.h"

registerMooseObject("CardinalApp", NekMassFluxWeightedSideAverage);

InputParameters
NekMassFluxWeightedSideAverage::validParams()
{
  InputParameters params = NekMassFluxWeightedSideIntegral::validParams();
  params.addClassDescription(
      "Compute mass flux weighted average of a field over a boundary in the NekRS mesh");
  return params;
}

NekMassFluxWeightedSideAverage::NekMassFluxWeightedSideAverage(const InputParameters & parameters)
  : NekMassFluxWeightedSideIntegral(parameters)
{
}

Real
NekMassFluxWeightedSideAverage::getValue() const
{
  return NekMassFluxWeightedSideIntegral::getValue() / nekrs::massFlowrate(_boundary, _pp_mesh);
}

#endif
