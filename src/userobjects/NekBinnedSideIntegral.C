#include "NekBinnedSideIntegral.h"
#include "CardinalUtils.h"
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
  nekrs::binnedGapVolume(_map_space_by_qp, &NekSideSpatialBinUserObject::bin,
    &NekSideSpatialBinUserObject::gapIndexAndDistance,
    this, num_bins(), _gap_thickness, _bin_volumes, _bin_counts);
}

Real
NekBinnedSideIntegral::spatialValue(const Point & p, const unsigned int & component) const
{
  // total bin index
  const auto & i = bin(p);

  // get the index of the gap
  auto local_indices = unrolledBin(i);
  auto gap_index = local_indices[_side_index];

  return _bin_values[i] * _velocity_bin_directions[gap_index](component);
}

void
NekBinnedSideIntegral::execute()
{
  // if the mesh is changing, re-compute the areas of the bins
  if (!_fixed_mesh)
    computeBinVolumes();

  if (_field == field::velocity_component)
  {
    nekrs::binnedSideIntegral(field::velocity_x, _map_space_by_qp, &NekSideSpatialBinUserObject::bin,
      &NekSideSpatialBinUserObject::gapIndexAndDistance,
      this, num_bins(), _gap_thickness, _bin_volumes, _bin_values_x);
    nekrs::binnedSideIntegral(field::velocity_y, _map_space_by_qp, &NekSideSpatialBinUserObject::bin,
      &NekSideSpatialBinUserObject::gapIndexAndDistance,
      this, num_bins(), _gap_thickness, _bin_volumes, _bin_values_y);
    nekrs::binnedSideIntegral(field::velocity_z, _map_space_by_qp, &NekSideSpatialBinUserObject::bin,
      &NekSideSpatialBinUserObject::gapIndexAndDistance,
      this, num_bins(), _gap_thickness, _bin_volumes, _bin_values_z);

    for (unsigned int i = 0; i < num_bins(); ++i)
    {
      auto local_indices = unrolledBin(i);
      auto gap_index = local_indices[_side_index];

      Point velocity(_bin_values_x[i], _bin_values_y[i], _bin_values_z[i]);
      _bin_values[i] = _velocity_bin_directions[gap_index] * velocity;
    }
  }
  else
    nekrs::binnedSideIntegral(_field, _map_space_by_qp, &NekSideSpatialBinUserObject::bin,
      &NekSideSpatialBinUserObject::gapIndexAndDistance,
      this, num_bins(), _gap_thickness, _bin_volumes, _bin_values);
}
