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

#include "SymmetryPointGenerator.h"
#include "MooseUtils.h"

SymmetryPointGenerator::SymmetryPointGenerator(const Point & point, const Point & normal) :
  _point(point),
  _normal(normal)
{
  Point zero(0.0, 0.0, 0.0);
  if (_normal.absolute_fuzzy_equals(zero))
    mooseError("Symmetry plane normal cannot have zero norm!");

  _unit_normal = _normal / _normal.norm();

  // equation of plane is ax + by + cz = d
  _a = _unit_normal(0);
  _b = _unit_normal(1);
  _c = _unit_normal(2);
  _d = _a * _point(0) + _b * _point(1) + _c * _point(2);
  _denominator = _a * _a + _b * _b + _c * _c;
}

bool
SymmetryPointGenerator::onPositiveSideOfPlane(const Point & p) const
{
  auto vector_from_plane_point = p - _point;
  return vector_from_plane_point * _normal > 0;
}

Point
SymmetryPointGenerator::reflectPointAcrossPlane(const Point & p) const
{
  if (onPositiveSideOfPlane(p))
  {
    // reflect onto the negative side of the plane - first, find the closest
    // point on the plane
    Real coeff = (_d - _unit_normal * p) / _denominator;
    return p + 2.0 * coeff * _unit_normal;
  }
  else
    return p;
}
