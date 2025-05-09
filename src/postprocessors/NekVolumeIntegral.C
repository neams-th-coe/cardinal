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

#include "NekVolumeIntegral.h"

registerMooseObject("CardinalApp", NekVolumeIntegral);

InputParameters
NekVolumeIntegral::validParams()
{
  InputParameters params = NekFieldPostprocessor::validParams();
  params.addClassDescription("Integral of a field over the NekRS volume mesh");
  return params;
}

NekVolumeIntegral::NekVolumeIntegral(const InputParameters & parameters)
  : NekFieldPostprocessor(parameters)
{
}

Real
NekVolumeIntegral::volume() const
{
  switch (_pp_mesh)
  {
    case nek_mesh::fluid:
      return nekrs::volume(nek_mesh::fluid);
    case nek_mesh::all:
      return nekrs::volume(nek_mesh::all);
    case nek_mesh::solid:
      return nekrs::volume(nek_mesh::all) - nekrs::volume(nek_mesh::fluid);
    default:
      mooseError("Unhandled NekMeshEnum in volume()!");
  }
}

Real
NekVolumeIntegral::getValue() const
{
  switch (_pp_mesh)
  {
    case nek_mesh::fluid:
      return getIntegralOnMesh(nek_mesh::fluid);
    case nek_mesh::all:
      return getIntegralOnMesh(nek_mesh::all);
    case nek_mesh::solid:
      return getIntegralOnMesh(nek_mesh::all) - getIntegralOnMesh(nek_mesh::fluid);
    default:
      mooseError("Unhandled NekMeshEnum in getValue()!");
  }
}

Real
NekVolumeIntegral::getIntegralOnMesh(const nek_mesh::NekMeshEnum & mesh) const
{
  Real vol = nekrs::volume(mesh);

  if (_field == field::velocity_component)
  {
    Real vx = nekrs::volumeIntegral(field::velocity_x, vol, mesh);
    Real vy = nekrs::volumeIntegral(field::velocity_y, vol, mesh);
    Real vz = nekrs::volumeIntegral(field::velocity_z, vol, mesh);
    Point velocity(vx, vy, vz);
    return _velocity_direction * velocity;
  }

  return nekrs::volumeIntegral(_field, vol, mesh);
}

#endif
