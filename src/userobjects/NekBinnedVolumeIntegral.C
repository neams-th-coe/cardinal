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
  _bin_volumes = (double *) calloc(num_bins(), sizeof(double));

  if (_fixed_mesh)
    nekrs::binnedVolume(_map_space_by_qp, &NekSpatialBinUserObject::bin, this, num_bins(), _bin_volumes);
}

NekBinnedVolumeIntegral::~NekBinnedVolumeIntegral()
{
  free(_bin_volumes);
}

void
NekBinnedVolumeIntegral::execute()
{
  // if the mesh is changing, re-compute the volumes of the bins
  if (!_fixed_mesh)
    nekrs::binnedVolume(_map_space_by_qp, &NekSpatialBinUserObject::bin, this, num_bins(), _bin_volumes);

  nekrs::binnedVolumeIntegral(_field, _map_space_by_qp, &NekSpatialBinUserObject::bin,
    this, num_bins(), _bin_volumes, _bin_values);
}
