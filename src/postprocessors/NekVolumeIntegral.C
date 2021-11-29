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

#include "NekVolumeIntegral.h"

registerMooseObject("CardinalApp", NekVolumeIntegral);

defineLegacyParams(NekVolumeIntegral);

InputParameters
NekVolumeIntegral::validParams()
{
  InputParameters params = NekFieldPostprocessor::validParams();
  params.addClassDescription("Compute the integral of a field over the NekRS mesh");
  return params;
}

NekVolumeIntegral::NekVolumeIntegral(const InputParameters & parameters) :
  NekFieldPostprocessor(parameters)
{
  if (_fixed_mesh)
    _volume = nekrs::volume();
}

Real
NekVolumeIntegral::getValue()
{
  if (!_fixed_mesh)
    _volume = nekrs::volume();

  if (_field == field::velocity_component)
  {
    Real vx = nekrs::volumeIntegral(field::velocity_x, _volume);
    Real vy = nekrs::volumeIntegral(field::velocity_y, _volume);
    Real vz = nekrs::volumeIntegral(field::velocity_z, _volume);
    Point velocity(vx, vy, vz);
    return _velocity_direction * velocity;
  }

  return nekrs::volumeIntegral(_field, _volume);
}
