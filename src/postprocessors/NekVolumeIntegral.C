#include "NekVolumeIntegral.h"

registerMooseObject("CardinalApp", NekVolumeIntegral);

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
