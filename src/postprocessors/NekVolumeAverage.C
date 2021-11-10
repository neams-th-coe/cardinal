#include "NekVolumeAverage.h"

registerMooseObject("CardinalApp", NekVolumeAverage);

defineLegacyParams(NekVolumeAverage);

InputParameters
NekVolumeAverage::validParams()
{
  InputParameters params = NekVolumeIntegral::validParams();
  params.addClassDescription("Compute a volume average of a specified field over the NekRS mesh");
  return params;
}

NekVolumeAverage::NekVolumeAverage(const InputParameters & parameters) :
  NekVolumeIntegral(parameters)
{
}

Real
NekVolumeAverage::getValue()
{
  return NekVolumeIntegral::getValue() / _volume;
}
