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
#include "GeometryUtils.h"
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
  params.addClassDescription("Maps from a point (x, y, z) to a new point that is either "
                             "mirror-symmetric or rotationally-symmetric from the point.");
  return params;
}

SymmetryPointGenerator::SymmetryPointGenerator(const InputParameters & params)
  : GeneralUserObject(params), _rotational_symmetry(isParamValid("rotation_axis"))
{
  checkJointParams(params, {"rotation_axis", "rotation_angle"}, "specifying rotational symmetry");

  _normal = geom_utils::unitVector(getParam<Point>("normal"), "normal");

  if (_rotational_symmetry)
  {
    const auto & angle = getParam<Real>("rotation_angle");

    _rotational_axis = geom_utils::unitVector(getParam<Point>("rotation_axis"), "rotation_axis");

    // the symmetry axis needs to be perpendicular to the plane normal
    if (!MooseUtils::absoluteFuzzyEqual(_rotational_axis * _normal, 0.0))
      paramError("rotation_axis", "The 'rotation_axis' must be perpendicular to the 'normal'!");

    // unit circle must be divisible by angle
    if (!MooseUtils::absoluteFuzzyEqual(fmod(360.0, angle), 0))
      paramError("rotation_angle",
                 "The unit circle must be evenly divisible by the 'rotation_angle'!");

    _angle = angle * M_PI / 180.0;

    _zero_theta = _normal.cross(_rotational_axis);
    _zero_theta = _zero_theta / _zero_theta.norm();

    _reflection_normal = geom_utils::rotatePointAboutAxis(_normal, -_angle / 2.0, _rotational_axis);
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
      pt = geom_utils::rotatePointAboutAxis(p, s * _angle, _rotational_axis);

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
