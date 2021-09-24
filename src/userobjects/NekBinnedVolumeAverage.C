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
  _bin_volumes = (double *) calloc(num_bins(), sizeof(double));
}

NekBinnedVolumeAverage::~NekBinnedVolumeAverage()
{
  free(_bin_volumes);
}

void
NekBinnedVolumeAverage::execute()
{
  // compute the integrals
  nekrs::binnedVolumeIntegral(_field, _map_space_by_qp, &NekSpatialBinUserObject::bin,
    this, num_bins(), _bin_values);

  // compute the volumes for normalization
  nekrs::binnedVolumeIntegral(field::unity, _map_space_by_qp, &NekSpatialBinUserObject::bin,
    this, num_bins(), _bin_volumes);

  for (unsigned int i = 0; i < num_bins(); ++i)
    _bin_values[i] /= _bin_volumes[i];
}
