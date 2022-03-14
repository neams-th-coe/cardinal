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

#include "NekBinnedSideAverage.h"

registerMooseObject("CardinalApp", NekBinnedSideAverage);

InputParameters
NekBinnedSideAverage::validParams()
{
  InputParameters params = NekBinnedSideIntegral::validParams();
  params.addClassDescription(
      "Compute the spatially-binned average of a field over a sideset of the NekRS mesh");
  return params;
}

NekBinnedSideAverage::NekBinnedSideAverage(const InputParameters & parameters)
  : NekBinnedSideIntegral(parameters)
{
}

void
NekBinnedSideAverage::execute()
{
  NekBinnedSideIntegral::execute();

  // divide by the bin area if accessible; otherwise, we know that there weren't any counts,
  // and the bin value will remain zero
  for (unsigned int i = 0; i < num_bins(); ++i)
    if (_bin_volumes[i] > 0.0)
      _bin_values[i] /= _bin_volumes[i];
}

#endif
