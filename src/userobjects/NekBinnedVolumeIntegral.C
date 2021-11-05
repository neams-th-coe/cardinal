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
  if (_fixed_mesh)
    computeBinVolumes();
}

void
NekBinnedVolumeIntegral::getBinVolumes()
{
  nekrs::binnedVolume(_map_space_by_qp, &NekSpatialBinUserObject::bin, this, num_bins(), _bin_volumes, _bin_counts);
}

void
NekBinnedVolumeIntegral::execute()
{
  // if the mesh is changing, re-compute the volumes of the bins and check the counts
  if (!_fixed_mesh)
    computeBinVolumes();

  nekrs::binnedVolumeIntegral(_field, _map_space_by_qp, &NekSpatialBinUserObject::bin,
    this, num_bins(), _bin_volumes, _bin_values);
}
