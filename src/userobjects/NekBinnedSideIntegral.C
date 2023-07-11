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

#include "NekBinnedSideIntegral.h"
#include "NekInterface.h"

registerMooseObject("CardinalApp", NekBinnedSideIntegral);

InputParameters
NekBinnedSideIntegral::validParams()
{
  InputParameters params = NekSideSpatialBinUserObject::validParams();
  params.addClassDescription(
      "Compute the spatially-binned side integral of a field over a boundary of the NekRS mesh");
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
  const auto & sgeo = nekrs::getSgeo();

  for (int i = 0; i < mesh->Nelements; ++i)
  {
    for (int j = 0; j < mesh->Nfaces; ++j)
    {
      int face_id = mesh->EToB[i * mesh->Nfaces + j];
      if (std::find(_boundary.begin(), _boundary.end(), face_id) != _boundary.end())
      {
        int offset = i * mesh->Nfaces * mesh->Nfp + j * mesh->Nfp;
        for (int v = 0; v < mesh->Nfp; ++v)
        {
          Point p = nekPoint(i, j, v);
          unsigned int b = bin(p);
          _bin_partial_values[b] += sgeo[mesh->Nsgeo * (offset + v) + WSJID];
          _bin_partial_counts[b]++;
        }
      }
    }
  }

  // sum across all processes
  MPI_Allreduce(
      _bin_partial_values, _bin_volumes, _n_bins, MPI_DOUBLE, MPI_SUM, platform->comm.mpiComm);
  MPI_Allreduce(
      _bin_partial_counts, _bin_counts, _n_bins, MPI_INT, MPI_SUM, platform->comm.mpiComm);

  // dimensionalize
  for (unsigned int i = 0; i < _n_bins; ++i)
    nekrs::dimensionalizeArea(_bin_volumes[i]);
}

void
NekBinnedSideIntegral::binnedSideIntegral(const field::NekFieldEnum & integrand,
                                          double * total_integral)
{
  resetPartialStorage();

  mesh_t * mesh = nekrs::entireMesh();
  double (*f)(int) = nekrs::solutionPointer(integrand);
  const auto & sgeo = nekrs::getSgeo();

  for (int i = 0; i < mesh->Nelements; ++i)
  {
    for (int j = 0; j < mesh->Nfaces; ++j)
    {
      int face_id = mesh->EToB[i * mesh->Nfaces + j];
      if (std::find(_boundary.begin(), _boundary.end(), face_id) != _boundary.end())
      {
        int offset = i * mesh->Nfaces * mesh->Nfp + j * mesh->Nfp;
        for (int v = 0; v < mesh->Nfp; ++v)
        {
          Point p = nekPoint(i, j, v);
          unsigned int b = bin(p);
          _bin_partial_values[b] +=
              f(mesh->vmapM[offset + v]) * sgeo[mesh->Nsgeo * (offset + v) + WSJID];
        }
      }
    }
  }

  // sum across all processes
  MPI_Allreduce(
      _bin_partial_values, total_integral, _n_bins, MPI_DOUBLE, MPI_SUM, platform->comm.mpiComm);

  // dimensionalize
  for (unsigned int i = 0; i < _n_bins; ++i)
    nekrs::dimensionalizeSideIntegral(integrand, _bin_volumes[i], total_integral[i]);
}

Real
NekBinnedSideIntegral::spatialValue(const Point & p, const unsigned int & component) const
{
  const auto & i = bin(p);
  return _bin_values[i] * _velocity_bin_directions[i](component);
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
      Point velocity(_bin_values_x[i], _bin_values_y[i], _bin_values_z[i]);
      _bin_values[i] = _velocity_bin_directions[i] * velocity;
    }
  }
  else
    binnedSideIntegral(_field, _bin_values);
}

void
NekBinnedSideIntegral::executeUserObject()
{
  computeIntegral();
}

#endif
