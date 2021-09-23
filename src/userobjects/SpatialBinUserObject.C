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
