#include "NekBinnedVolumeIntegral.h"
#include "NekInterface.h"

registerMooseObject("CardinalApp", NekBinnedVolumeIntegral);

InputParameters
NekBinnedVolumeIntegral::validParams()
{
  InputParameters params = NekVolumeSpatialBinUserObject::validParams();
  params.addClassDescription("Compute the spatially-binned volume integral of a field over the NekRS mesh");
  return params;
}

NekBinnedVolumeIntegral::NekBinnedVolumeIntegral(const InputParameters & parameters)
  : NekVolumeSpatialBinUserObject(parameters)
{
  if (_fixed_mesh)
    computeBinVolumes();
}

void
NekBinnedVolumeIntegral::getBinVolumes()
{
  nekrs::binnedVolume(_map_space_by_qp, &NekVolumeSpatialBinUserObject::bin, this, num_bins(), _bin_volumes, _bin_counts);
}

Real
NekBinnedVolumeIntegral::spatialValue(const Point & p, const unsigned int & component) const
{
  const auto & i = bin(p);
  return _bin_values[i] * _velocity_bin_directions[i](component);
}

void
NekBinnedVolumeIntegral::execute()
{
  // if the mesh is changing, re-compute the volumes of the bins and check the counts
  if (!_fixed_mesh)
    computeBinVolumes();

  if (_field == field::velocity_component)
  {
    nekrs::binnedVolumeIntegral(field::velocity_x, _map_space_by_qp, &NekVolumeSpatialBinUserObject::bin,
      this, num_bins(), _bin_volumes, _bin_values_x);
    nekrs::binnedVolumeIntegral(field::velocity_y, _map_space_by_qp, &NekVolumeSpatialBinUserObject::bin,
      this, num_bins(), _bin_volumes, _bin_values_y);
    nekrs::binnedVolumeIntegral(field::velocity_z, _map_space_by_qp, &NekVolumeSpatialBinUserObject::bin,
      this, num_bins(), _bin_volumes, _bin_values_z);

    for (unsigned int i = 0; i < num_bins(); ++i)
    {
      Point velocity(_bin_values_x[i], _bin_values_y[i], _bin_values_z[i]);
      _bin_values[i] = _velocity_bin_directions[i] * velocity;
    }
  }
  else
  {
    nekrs::binnedVolumeIntegral(_field, _map_space_by_qp, &NekVolumeSpatialBinUserObject::bin,
      this, num_bins(), _bin_volumes, _bin_values);
  }
}
