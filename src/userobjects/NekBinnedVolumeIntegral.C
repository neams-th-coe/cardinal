#include "NekBinnedVolumeIntegral.h"
#include "NekInterface.h"

registerMooseObject("CardinalApp", NekBinnedVolumeIntegral);

InputParameters
NekBinnedVolumeIntegral::validParams()
{
  InputParameters params = NekSpatialBinUserObject::validParams();
  params.addClassDescription("Compute the spatially-binned volume integral of a field over the NekRS mesh");
  return params;
}

NekBinnedVolumeIntegral::NekBinnedVolumeIntegral(const InputParameters & parameters)
  : NekSpatialBinUserObject(parameters)
{
}

void
NekBinnedVolumeIntegral::execute()
{
  nekrs::binnedVolumeIntegral(_field, _map_space_by_qp, &NekSpatialBinUserObject::bin,
    this, num_bins(), _bin_values);
}
