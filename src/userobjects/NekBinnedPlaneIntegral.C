/********************************************************************/
/*                  SOFTWARE COPYRIGHT NOTIFICATION                 */
/*                             Cardinal                             */
/*                                                                  */
/*                  (c) 2021 UChicago Argonne, LLC                  */
/*                        ALL RIGHTS RESERVED                       */
/*                                                                  */
/*                 Prepared by UChicago Argonne, LLC                */
/*               Under Contract No. DE-AC02-06CH11357               */
/*                With the U. S. Department of Energy               */
/*                                                                  */
/*             Prepared by Battelle Energy Alliance, LLC            */
/*               Under Contract No. DE-AC07-05ID14517               */
/*                With the U. S. Department of Energy               */
/*                                                                  */
/*                 See LICENSE for full restrictions                */
/********************************************************************/

#ifdef ENABLE_NEK_COUPLING

#include "NekBinnedPlaneIntegral.h"
#include "NekInterface.h"

registerMooseObject("CardinalApp", NekBinnedPlaneIntegral);

InputParameters
NekBinnedPlaneIntegral::validParams()
{
  InputParameters params = NekPlaneSpatialBinUserObject::validParams();
  params.addClassDescription(
      "Compute the spatially-binned side integral of a field over the NekRS mesh");
  return params;
}

NekBinnedPlaneIntegral::NekBinnedPlaneIntegral(const InputParameters & parameters)
  : NekPlaneSpatialBinUserObject(parameters)
{
  if (_fixed_mesh)
    computeBinVolumes();
}

void
NekBinnedPlaneIntegral::getBinVolumes()
{
  resetPartialStorage();
  mesh_t * mesh = nekrs::entireMesh();
  const auto & vgeo = nekrs::getVgeo();

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
        _bin_partial_values[b] += vgeo[mesh->Nvgeo * offset + v + mesh->Np * JWID];
        _bin_partial_counts[b]++;
      }
    }
  }

  // sum across all processes
  MPI_Allreduce(
      _bin_partial_values, _bin_volumes, _n_bins, MPI_DOUBLE, MPI_SUM, platform->comm.mpiComm);
  MPI_Allreduce(
      _bin_partial_counts, _bin_counts, _n_bins, MPI_INT, MPI_SUM, platform->comm.mpiComm);

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
NekBinnedPlaneIntegral::binnedPlaneIntegral(const field::NekFieldEnum & integrand,
                                            double * total_integral)
{
  resetPartialStorage();

  mesh_t * mesh = nekrs::entireMesh();
  double (*f)(int) = nekrs::solutionPointer(integrand);
  const auto & vgeo = nekrs::getVgeo();

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
        _bin_partial_values[b] +=
            f(offset + v) * vgeo[mesh->Nvgeo * offset + v + mesh->Np * JWID];
      }
    }
  }

  // sum across all processes
  MPI_Allreduce(
      _bin_partial_values, total_integral, _n_bins, MPI_DOUBLE, MPI_SUM, platform->comm.mpiComm);

  for (unsigned int i = 0; i < _n_bins; ++i)
  {
    // some bins require dividing by a different value, depending on the bin type
    const auto local_bins = unrolledBin(i);
    total_integral[i] *= _side_bin->adjustBinValue(local_bins[_side_index]);

    nekrs::dimensionalizeVolumeIntegral(integrand, _bin_volumes[i], total_integral[i]);
  }
}

Real
NekBinnedPlaneIntegral::spatialValue(const Point & p, const unsigned int & component) const
{
  // total bin index
  const auto & i = bin(p);

  // get the index of the gap
  auto local_indices = unrolledBin(i);
  auto gap_index = local_indices[_side_index];

  return _bin_values[i] * _velocity_bin_directions[gap_index](component);
}

void
NekBinnedPlaneIntegral::computeIntegral()
{
  // if the mesh is changing, re-compute the areas of the bins
  if (!_fixed_mesh)
    computeBinVolumes();

  if (_field == field::velocity_component)
  {
    binnedPlaneIntegral(field::velocity_x, _bin_values_x);
    binnedPlaneIntegral(field::velocity_y, _bin_values_y);
    binnedPlaneIntegral(field::velocity_z, _bin_values_z);

    for (unsigned int i = 0; i < num_bins(); ++i)
    {
      auto local_indices = unrolledBin(i);
      auto gap_index = local_indices[_side_index];

      Point velocity(_bin_values_x[i], _bin_values_y[i], _bin_values_z[i]);
      _bin_values[i] = _velocity_bin_directions[gap_index] * velocity;
    }
  }
  else
    binnedPlaneIntegral(_field, _bin_values);
}

void
NekBinnedPlaneIntegral::executeUserObject()
{
  computeIntegral();

  // correct the values to areas
  for (unsigned int i = 0; i < _n_bins; ++i)
    _bin_values[i] /= _gap_thickness;
}

#endif
