#include "NekSideAverage.h"

registerMooseObject("CardinalApp", NekSideAverage);

InputParameters
NekSideAverage::validParams()
{
  InputParameters params = NekSideIntegral::validParams();
  params.addClassDescription("Compute the average of a field over a boundary of the NekRS mesh");
  return params;
}

NekSideAverage::NekSideAverage(const InputParameters & parameters) :
  NekSideIntegral(parameters)
{
  if (_fixed_mesh)
    _area = nekrs::area(_boundary);
}

Real
NekSideAverage::getValue()
{
  Real area = _fixed_mesh ? _area : nekrs::area(_boundary);
  return NekSideIntegral::getValue() / area;
}
