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

#include "NekBinnedPlaneAverage.h"

registerMooseObject("CardinalApp", NekBinnedPlaneAverage);

InputParameters
NekBinnedPlaneAverage::validParams()
{
  InputParameters params = NekPlaneSpatialBinUserObject::validParams();
  params.addClassDescription("Compute the spatially-binned side average of a field over the NekRS mesh");
  return params;
}

NekBinnedPlaneAverage::NekBinnedPlaneAverage(const InputParameters & parameters)
  : NekBinnedPlaneIntegral(parameters)
{
}

void
NekBinnedPlaneAverage::execute()
{
  computeIntegral();

  // divide by the bin volume if accessible; otherwise, we know that there weren't any counts,
  // and the bin value will remain zero
  for (unsigned int i = 0; i < num_bins(); ++i)
    if (_bin_volumes[i] > 0.0)
      _bin_values[i] /= _bin_volumes[i];
}
