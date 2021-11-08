#include "NekBinnedVolumeAverage.h"
#include "NekInterface.h"

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
