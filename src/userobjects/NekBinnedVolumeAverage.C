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

#include "NekBinnedVolumeAverage.h"

registerMooseObject("CardinalApp", NekBinnedVolumeAverage);

InputParameters
NekBinnedVolumeAverage::validParams()
{
  InputParameters params = NekBinnedVolumeIntegral::validParams();
  params.addClassDescription("Compute the spatially-binned volume average of a field over the NekRS mesh");
  return params;
}

NekBinnedVolumeAverage::NekBinnedVolumeAverage(const InputParameters & parameters)
  : NekBinnedVolumeIntegral(parameters)
{
}

void
NekBinnedVolumeAverage::execute()
{
  NekBinnedVolumeIntegral::execute();

  for (unsigned int i = 0; i < num_bins(); ++i)
    _bin_values[i] /= _bin_volumes[i];
}
