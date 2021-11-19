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
  resetPartialStorage();
  mesh_t * mesh = nekrs::entireMesh();

  for (int k = 0; k < mesh->Nelements; ++k)
  {
    int offset = k * mesh->Np;
    for (int v = 0; v < mesh->Np; ++v)
    {
      Point p = nekPoint(k, v);
      unsigned int gap_bin;
      double distance;
      gapIndexAndDistance(p, gap_bin, distance);

      if (distance < _gap_thickness / 2.0)
      {
        unsigned int b = bin(p);
        _bin_partial_values[b] += mesh->vgeo[mesh->Nvgeo * offset + v + mesh->Np * JWID];
        _bin_partial_counts[b]++;
      }
    }
  }

  // sum across all processes
  MPI_Allreduce(_bin_partial_values, _bin_volumes, _n_bins, MPI_DOUBLE, MPI_SUM, platform->comm.mpiComm);
  MPI_Allreduce(_bin_partial_counts, _bin_counts, _n_bins, MPI_INT, MPI_SUM, platform->comm.mpiComm);

  for (unsigned int i = 0; i < _n_bins; ++i)
  {
    // some bins require dividing by a different value, depending on the bin type
    const auto local_bins = unrolledBin(i);
    _bin_volumes[i] *= _side_bin->adjustBinValue(local_bins[_side_index]);

    // dimensionalize
    nekrs::dimensionalizeVolume(_bin_volumes[i]);
  }
}

void
NekBinnedSideIntegral::binnedSideIntegral(const field::NekFieldEnum & integrand, double * total_integral)
{
  resetPartialStorage();

  mesh_t * mesh = nekrs::entireMesh();
  double (*f) (int) = nekrs::solution::solutionPointer(integrand);

  for (int k = 0; k < mesh->Nelements; ++k)
  {
    int offset = k * mesh->Np;
    for (int v = 0; v < mesh->Np; ++v)
    {
      Point p = nekPoint(k, v);

      unsigned int gap_bin;
      double distance;
      gapIndexAndDistance(p, gap_bin, distance);

      if (distance < _gap_thickness / 2.0)
      {
        unsigned int b = bin(p);
        _bin_partial_values[b] += f(offset + v) * mesh->vgeo[mesh->Nvgeo * offset + v + mesh->Np * JWID];
      }
    }
  }

  // sum across all processes
  MPI_Allreduce(_bin_partial_values, total_integral, _n_bins, MPI_DOUBLE, MPI_SUM, platform->comm.mpiComm);

  for (unsigned int i = 0; i < _n_bins; ++i)
  {
    // some bins require dividing by a different value, depending on the bin type
    const auto local_bins = unrolledBin(i);
    total_integral[i] *= _side_bin->adjustBinValue(local_bins[_side_index]);

    nekrs::dimensionalizeVolumeIntegral(integrand, _bin_volumes[i], total_integral[i]);
  }
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
NekBinnedSideIntegral::computeIntegral()
{
  // if the mesh is changing, re-compute the areas of the bins
  if (!_fixed_mesh)
    computeBinVolumes();

  if (_field == field::velocity_component)
  {
    binnedSideIntegral(field::velocity_x, _bin_values_x);
    binnedSideIntegral(field::velocity_y, _bin_values_y);
    binnedSideIntegral(field::velocity_z, _bin_values_z);

    for (unsigned int i = 0; i < num_bins(); ++i)
    {
      auto local_indices = unrolledBin(i);
      auto gap_index = local_indices[_side_index];

      Point velocity(_bin_values_x[i], _bin_values_y[i], _bin_values_z[i]);
      _bin_values[i] = _velocity_bin_directions[gap_index] * velocity;
    }
  }
  else
    binnedSideIntegral(_field, _bin_values);
}

void
NekBinnedSideIntegral::execute()
{
  computeIntegral();

  // correct the values to areas
  for (unsigned int i = 0; i < _n_bins; ++i)
    _bin_values[i] /= _gap_thickness;
}
