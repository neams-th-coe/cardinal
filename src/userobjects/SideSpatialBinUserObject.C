#include "SideSpatialBinUserObject.h"

InputParameters
SideSpatialBinUserObject::validParams()
{
  InputParameters params = SpatialBinUserObject::validParams();
  return params;
}

SideSpatialBinUserObject::SideSpatialBinUserObject(const InputParameters & parameters)
  : SpatialBinUserObject(parameters)
{
}
