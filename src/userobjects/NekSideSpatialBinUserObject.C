#include "NekSideSpatialBinUserObject.h"

InputParameters
NekSideSpatialBinUserObject::validParams()
{
  InputParameters params = NekSpatialBinUserObject::validParams();
  params.addRequiredRangeCheckedParam<Real>("gap_thickness", "gap_thickness > 0.0",
    "thickness of gap region for which to accept contributions to the side integral over "
    "the gap, expressed in the same units as the mesh.");
  return params;
}

NekSideSpatialBinUserObject::NekSideSpatialBinUserObject(const InputParameters & parameters)
  : NekSpatialBinUserObject(parameters),
    _gap_thickness(getParam<Real>("gap_thickness"))
{
  // we need to enforce that there is only one side distribution, because side
  // distributions defined in orthogonal directions don't ever overlap with one another
  unsigned int num_side_distributions = 0;

  for (auto & uo : _bins)
  {
    const SideSpatialBinUserObject * side = dynamic_cast<const SideSpatialBinUserObject *>(uo);
    if (side)
    {
      ++num_side_distributions;
      _side_bin = side;
    }
  }

  if (num_side_distributions != 1)
    mooseError("'" + name() + "' requires exactly one bin distribution "
      "to be a side distribution; you have specified: " + Moose::stringify(num_side_distributions) +
      "\noptions: HexagonalSubchannelGapBin");
}

Real
NekSideSpatialBinUserObject::distanceFromGap(const Point & point, const unsigned int & gap_index) const
{
  return _side_bin->distanceFromGap(point, gap_index);
}

unsigned int
NekSideSpatialBinUserObject::gapIndex(const Point & point) const
{
  return _side_bin->gapIndex(point);
}

void
NekSideSpatialBinUserObject::gapIndexAndDistance(const Point & point, unsigned int & index,  Real & distance) const
{
  _side_bin->gapIndexAndDistance(point, index, distance);
}
