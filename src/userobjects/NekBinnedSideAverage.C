#include "NekBinnedSideAverage.h"

registerMooseObject("CardinalApp", NekBinnedSideAverage);

InputParameters
NekBinnedSideAverage::validParams()
{
  InputParameters params = NekSideSpatialBinUserObject::validParams();
  params.addClassDescription("Compute the spatially-binned side average of a field over the NekRS mesh");
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

  for (unsigned int i = 0; i < num_bins(); ++i)
    _bin_values[i] /= _bin_volumes[i];
}
