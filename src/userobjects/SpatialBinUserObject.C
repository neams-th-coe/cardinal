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

#include "SpatialBinUserObject.h"
#include "BinUtility.h"

InputParameters
SpatialBinUserObject::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  return params;
}

SpatialBinUserObject::SpatialBinUserObject(const InputParameters & parameters)
  : GeneralUserObject(parameters)
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
  return bin_utility::linearBin(pt, bounds);
}
