#include "NekSideIntegral.h"

registerMooseObject("CardinalApp", NekSideIntegral);

InputParameters
NekSideIntegral::validParams()
{
  InputParameters params = NekSideFieldPostprocessor::validParams();
  params.addClassDescription("Compute the integral of a field over a boundary of the NekRS mesh");
  return params;
}

NekSideIntegral::NekSideIntegral(const InputParameters & parameters) :
  NekSideFieldPostprocessor(parameters)
{
}

Real
NekSideIntegral::getValue()
{
  if (_field == field::velocity_component)
  {
    Real vx = nekrs::sideIntegral(_boundary, field::velocity_x);
    Real vy = nekrs::sideIntegral(_boundary, field::velocity_y);
    Real vz = nekrs::sideIntegral(_boundary, field::velocity_z);
    Point velocity(vx, vy, vz);
    return _velocity_direction * velocity;;
  }

  return nekrs::sideIntegral(_boundary, _field);
}
