#include "NekVolumeSpatialBinUserObject.h"
#include "SideSpatialBinUserObject.h"

InputParameters
NekVolumeSpatialBinUserObject::validParams()
{
  InputParameters params = NekSpatialBinUserObject::validParams();
  return params;
}

NekVolumeSpatialBinUserObject::NekVolumeSpatialBinUserObject(const InputParameters & parameters)
  : NekSpatialBinUserObject(parameters)
{
  // we need to enforce that there are only volume distributions
  unsigned int num_side_distributions = 0;

  for (auto & uo : _bins)
  {
    const SideSpatialBinUserObject * side_bin = dynamic_cast<const SideSpatialBinUserObject *>(uo);
    if (side_bin)
      ++num_side_distributions;
  }

  if (num_side_distributions != 0)
    mooseError("This user object requires all bins to be volume distributions; you have specified " +
      Moose::stringify(num_side_distributions) + " side distributions." +
      "\noptions: HexagonalSubchannelBin, LayeredBin, RadialBin");
}
