#include "SpatialBinUserObject.h"

InputParameters
SpatialBinUserObject::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  return params;
}

SpatialBinUserObject::SpatialBinUserObject(const InputParameters & parameters)
  : ThreadedGeneralUserObject(parameters)
{
}

Real
SpatialBinUserObject::spatialValue(const Point & p) const
{
  return bin(p);
}

unsigned int
SpatialBinUserObject::binFromBounds(const Real & pt, const std::vector<Real> & bounds) const
{
  // This finds the first entry in the vector that is larger than what we're looking for
  std::vector<Real>::const_iterator one_higher = std::upper_bound(bounds.begin(), bounds.end(), pt);

  if (one_higher == bounds.end())
    return static_cast<unsigned int>(bounds.size() - 2);
  else if (one_higher == bounds.begin())
    return 0;
  else
    return static_cast<unsigned int>(std::distance(bounds.begin(), one_higher - 1));
}
