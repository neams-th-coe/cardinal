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

#include "NekSideAverage.h"

registerMooseObject("CardinalApp", NekSideAverage);

InputParameters
NekSideAverage::validParams()
{
  InputParameters params = NekSideIntegral::validParams();
  params.addClassDescription("Compute the average of a field over a boundary of the NekRS mesh");
  return params;
}

NekSideAverage::NekSideAverage(const InputParameters & parameters) : NekSideIntegral(parameters)
{
}

Real
NekSideAverage::getValue()
{
  _area = nekrs::area(_boundary, _pp_mesh);
  return NekSideIntegral::getValue() / _area;
}

#endif
