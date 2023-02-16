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
#include "GeometryUtility.h"
#include "UserErrorChecking.h"

registerMooseObject("CardinalApp", SymmetryPointGenerator);

InputParameters
SymmetryPointGenerator::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params.addRequiredParam<Point>("normal", "Normal of the symmetry plane");
  params.addParam<Point>("rotation_axis",
                         "If rotationally symmetric, the axis about which to rotate. "
                         "If not specified, then the geometry is mirror-symmetric.");
  params.addRangeCheckedParam<Real>(
      "rotation_angle",
      "rotation_angle > 0 & rotation_angle <= 180",
      "If rotationally symmetric, the angle (degrees) from the 'normal' plane "
      "through which to rotate to form the symmetric wedge. If not specified, then the"
      "geometry is mirror-symmetric.");
  return params;
}

SymmetryPointGenerator::SymmetryPointGenerator(const InputParameters & params)
  : ThreadedGeneralUserObject(params),
    _rotational_symmetry(isParamValid("rotation_axis"))
{
  checkJointParams(params, {"rotation_axis", "rotation_angle"}, "specifying rotational symmetry");

  auto n = getParam<Point>("normal");
  Point zero(0.0, 0.0, 0.0);
  if (n.absolute_fuzzy_equals(zero))
    mooseError("The 'normal' cannot have zero norm!");

  _normal = n / n.norm();

  if (_rotational_symmetry)
  {
    const auto & axis = getParam<Point>("rotation_axis");
    const auto & angle = getParam<Real>("rotation_angle");

    // symmetry axis cannot be zero norm
    Point zero(0.0, 0.0, 0.0);
    if (axis.absolute_fuzzy_equals(zero))
      mooseError("The 'rotation_axis' cannot have zero norm!");

    // the symmetry axis needs to be perpendicular to the plane normal
    if (!MooseUtils::absoluteFuzzyEqual(axis * _normal, 0.0))
      mooseError("The 'rotation_axis' must be perpendicular to the 'normal'!");

    _rotational_axis = axis / axis.norm();

    // unit circle must be divisible by angle
    if (!MooseUtils::absoluteFuzzyEqual(fmod(360.0, angle), 0))
      mooseError("The unit circle must be evenly divisible by the 'rotation_angle'!");

    _angle = angle * M_PI / 180.0;

    _zero_theta = _normal.cross(_rotational_axis);
    _zero_theta = _zero_theta / _zero_theta.norm();

    _reflection_normal = geom_utility::rotatePointAboutAxis(_normal, -_angle / 2.0, _rotational_axis);
    _reflection_normal = _reflection_normal / _reflection_normal.norm();
  }
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
      pt = geom_utility::rotatePointAboutAxis(p, s * _angle, _rotational_axis);

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