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
  params.makeParamNotRequired("field");
  params.set<MooseEnum>("field", true) = "unity";
  params.setDocString("function", "MOOSE function to evaluate and integrate on the native NekRS GLL mesh");
  params.addClassDescription("Integral of either a NekRS field or a MOOSE function over the NekRS volume mesh");
  return params;
}

NekVolumeIntegral::NekVolumeIntegral(const InputParameters & parameters)
  : NekFieldPostprocessor(parameters)
{
  const bool field_is_set = parameters.isParamSetByUser("field");
  const bool function_is_set = parameters.isParamSetByUser("function");

  if (field_is_set == function_is_set)
    mooseError("NekVolumeIntegral requires exactly one of 'field' or 'function'.");
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
  const Real vol = nekrs::volume(mesh);

  if (_function)
    return nekrs::volumeIntegral(_field, vol, mesh, _function, _t);

  if (_field == field::velocity_component)
  {
    const Real vx = nekrs::volumeIntegral(field::velocity_x, vol, mesh, nullptr, _t);
    const Real vy = nekrs::volumeIntegral(field::velocity_y, vol, mesh, nullptr, _t);
    const Real vz = nekrs::volumeIntegral(field::velocity_z, vol, mesh, nullptr, _t);

    const Point velocity(vx, vy, vz);

    return _velocity_direction * velocity;
  }

  return nekrs::volumeIntegral(_field, vol, mesh, nullptr, _t);
}

#endif
