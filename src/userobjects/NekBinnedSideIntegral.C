#include "NekBinnedSideIntegral.h"
#include "NekInterface.h"

registerMooseObject("CardinalApp", NekBinnedSideIntegral);

InputParameters
NekBinnedSideIntegral::validParams()
{
  InputParameters params = NekSideSpatialBinUserObject::validParams();
  params.addClassDescription("Compute the spatially-binned side integral of a field over the NekRS mesh");
  return params;
}

NekBinnedSideIntegral::NekBinnedSideIntegral(const InputParameters & parameters)
  : NekSideSpatialBinUserObject(parameters)
{
  if (_fixed_mesh)
    computeBinVolumes();
}

void
NekBinnedSideIntegral::getBinVolumes()
{
  nekrs::binnedGapVolume(_map_space_by_qp, &NekSpatialBinUserObject::bin,
    &NekSideSpatialBinUserObject::gapIndexAndDistance,
    this, num_bins(), _gap_thickness, _bin_volumes, _bin_counts);
}

void
NekBinnedSideIntegral::execute()
{
  // if the mesh is changing, re-compute the areas of the bins
  if (!_fixed_mesh)
    computeBinVolumes();

  nekrs::binnedSideIntegral(_field, _map_space_by_qp, &NekSideSpatialBinUserObject::bin,
    &NekSideSpatialBinUserObject::gapIndexAndDistance,
    this, num_bins(), _gap_thickness, _bin_volumes, _bin_values);
}
