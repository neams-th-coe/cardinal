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
  params.addClassDescription("Compute the integral of a field over the NekRS mesh");
  return params;
}

NekVolumeIntegral::NekVolumeIntegral(const InputParameters & parameters)
  : NekFieldPostprocessor(parameters)
{
}

Real
NekVolumeIntegral::getValue()
{
  _volume = nekrs::volume(_pp_mesh);

  if (_field == field::velocity_component)
  {
    Real vx = nekrs::volumeIntegral(field::velocity_x, _volume, _pp_mesh);
    Real vy = nekrs::volumeIntegral(field::velocity_y, _volume, _pp_mesh);
    Real vz = nekrs::volumeIntegral(field::velocity_z, _volume, _pp_mesh);
    Point velocity(vx, vy, vz);
    return _velocity_direction * velocity;
  }

  return nekrs::volumeIntegral(_field, _volume, _pp_mesh);
}

#endif
