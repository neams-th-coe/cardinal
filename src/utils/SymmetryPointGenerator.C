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
#include "math.h"

SymmetryPointGenerator::SymmetryPointGenerator(const Point & normal) : _rotational_symmetry(false)
{
  Point zero(0.0, 0.0, 0.0);
  if (normal.absolute_fuzzy_equals(zero))
    mooseError("The 'symmetry_plane_normal' cannot have zero norm!");

  _normal = normal / normal.norm();
}

void
SymmetryPointGenerator::initializeAngularSymmetry(const Point & axis, const Real & angle)
{
  _rotational_symmetry = true;

  // symmetry axis cannot be zero norm
  Point zero(0.0, 0.0, 0.0);
  if (axis.absolute_fuzzy_equals(zero))
    mooseError("The 'symmetry_axis' cannot have zero norm!");

  // the symmetry axis needs to be perpendicular to the plane normal
  if (!MooseUtils::absoluteFuzzyEqual(axis * _normal, 0.0))
    mooseError("The 'symmetry_axis' must be perpendicular to the 'symmetry_plane_normal'!");

  _rotational_axis = axis / axis.norm();

  // unit circle must be divisible by angle
  if (!MooseUtils::absoluteFuzzyEqual(fmod(360.0, angle), 0))
    mooseError("The unit circle must be divisible by the 'symmetry_angle'!");

  _angle = angle * M_PI / 180.0;

  _zero_theta = _normal.cross(_rotational_axis);
  _zero_theta = _zero_theta / _zero_theta.norm();

  _reflection_normal = rotatePointAboutAxis(_normal, -_angle / 2.0, _rotational_axis);
  _reflection_normal = _reflection_normal / _reflection_normal.norm();
}

bool
SymmetryPointGenerator::onPositiveSideOfPlane(const Point & p, const Point & normal) const
{
  return p * normal > 0;
}

Point
SymmetryPointGenerator::reflectPointAcrossPlane(const Point & p, const Point & normal) const
{
  Real coeff = -normal * p;
  return p + 2.0 * coeff * normal;
}

Point
SymmetryPointGenerator::rotatePointAboutAxis(const Point & p,
                                             const Real & angle,
                                             const Point & axis) const
{
  Real cos_theta = cos(angle);
  Real sin_theta = sin(angle);

  Point pt;
  Real xy = axis(0) * axis(1);
  Real xz = axis(0) * axis(2);
  Real yz = axis(1) * axis(2);

  Point x_op(cos_theta + axis(0) * axis(0) * (1.0 - cos_theta),
             xy * (1.0 - cos_theta) - axis(2) * sin_theta,
             xz * (1.0 - cos_theta) + axis(1) * sin_theta);

  Point y_op(xy * (1.0 - cos_theta) + axis(2) * sin_theta,
             cos_theta + axis(1) * axis(1) * (1.0 - cos_theta),
             yz * (1.0 - cos_theta) - axis(0) * sin_theta);

  Point z_op(xz * (1.0 - cos_theta) - axis(1) * sin_theta,
             yz * (1.0 - cos_theta) + axis(0) * sin_theta,
             cos_theta + axis(2) * axis(2) * (1.0 - cos_theta));

  pt(0) = x_op * p;
  pt(1) = y_op * p;
  pt(2) = z_op * p;
  return pt;
}

int
SymmetryPointGenerator::sector(const Point & p) const
{
  Real theta = acos(p * _zero_theta / p.norm());
  if (onPositiveSideOfPlane(p, _normal))
    theta = 2.0 * M_PI - theta;

  return theta / _angle;
}

Point
SymmetryPointGenerator::transformPoint(const Point & p) const
{
  Point pt = p;

  if (_rotational_symmetry)
  {
    // first, find the closest point on the plane
    Real coeff = -_rotational_axis * p;
    Point vec_to_pt = p + coeff * _rotational_axis;

    // get the sector - we only need to do a transformation if not in the first sector
    int s = sector(vec_to_pt);
    if (s != 0)
    {
      pt = rotatePointAboutAxis(p, s * _angle, _rotational_axis);

      // if the sector was odd, we also need to reflect the point about an axis
      // halfway between the symmetry plane and the zero-theta line
      if (s % 2 != 0)
        pt = reflectPointAcrossPlane(pt, _reflection_normal);
    }
  }
  else
  {
    if (onPositiveSideOfPlane(pt, _normal))
      pt = reflectPointAcrossPlane(pt, _normal);
  }

  return pt;
}
