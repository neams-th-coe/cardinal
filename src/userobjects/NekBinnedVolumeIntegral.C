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

#include "NekBinnedVolumeIntegral.h"
#include "CardinalUtils.h"
#include "NekInterface.h"

registerMooseObject("CardinalApp", NekBinnedVolumeIntegral);

InputParameters
NekBinnedVolumeIntegral::validParams()
{
  InputParameters params = NekVolumeSpatialBinUserObject::validParams();
  params.addClassDescription(
      "Compute the spatially-binned volume integral of a field over the NekRS mesh");
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
  resetPartialStorage();

  mesh_t * mesh = nekrs::entireMesh();
  const auto & vgeo = nekrs::getVgeo();
  for (int k = 0; k < mesh->Nelements; ++k)
  {
    int offset = k * mesh->Np;
    for (int v = 0; v < mesh->Np; ++v)
    {
      Point p = nekPoint(k, v);
      unsigned int b = bin(p);
      _bin_partial_values[b] += vgeo[mesh->Nvgeo * offset + v + mesh->Np * JWID];
      _bin_partial_counts[b]++;
    }
  }

  // sum across all processes
  MPI_Allreduce(
      _bin_partial_values, _bin_volumes, _n_bins, MPI_DOUBLE, MPI_SUM, platform->comm.mpiComm);
  MPI_Allreduce(
      _bin_partial_counts, _bin_counts, _n_bins, MPI_INT, MPI_SUM, platform->comm.mpiComm);

  // dimensionalize
  for (unsigned int i = 0; i < _n_bins; ++i)
    nekrs::dimensionalizeVolume(_bin_volumes[i]);
}

Real
NekBinnedVolumeIntegral::spatialValue(const Point & p, const unsigned int & component) const
{
  const auto & i = bin(p);
  return _bin_values[i] * _velocity_bin_directions[i](component);
}

void
NekBinnedVolumeIntegral::binnedVolumeIntegral(const field::NekFieldEnum & integrand,
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
      unsigned int b = bin(p);
      _bin_partial_values[b] +=
          f(offset + v) * vgeo[mesh->Nvgeo * offset + v + mesh->Np * JWID];
    }
  }

  // sum across all processes
  MPI_Allreduce(
      _bin_partial_values, total_integral, _n_bins, MPI_DOUBLE, MPI_SUM, platform->comm.mpiComm);

  for (unsigned int i = 0; i < _n_bins; ++i)
    nekrs::dimensionalizeVolumeIntegral(integrand, _bin_volumes[i], total_integral[i]);
}

void
NekBinnedVolumeIntegral::executeUserObject()
{
  // if the mesh is changing, re-compute the volumes of the bins and check the counts
  if (!_fixed_mesh)
    computeBinVolumes();

  if (_field == field::velocity_component)
  {
    binnedVolumeIntegral(field::velocity_x, _bin_values_x);
    binnedVolumeIntegral(field::velocity_y, _bin_values_y);
    binnedVolumeIntegral(field::velocity_z, _bin_values_z);

    for (unsigned int i = 0; i < num_bins(); ++i)
    {
      Point velocity(_bin_values_x[i], _bin_values_y[i], _bin_values_z[i]);
      _bin_values[i] = _velocity_bin_directions[i] * velocity;
    }
  }
  else
    binnedVolumeIntegral(_field, _bin_values);
}

#endif
