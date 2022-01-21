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

#pragma once

#include "Moose.h"
#include "MooseTypes.h"
#include "libmesh/point.h"

/**
 * Class that rotates/reflects a point about symmetry planes; the origin
 * for reflection and rotation is (0.0, 0.0, 0.0)
 */
class SymmetryPointGenerator
{
public:
  SymmetryPointGenerator(const Point & normal);

  /**
   * Initialize information needed to transform points with angular symmetry
   * @param[in] axis axis of symmetry
   * @param[in] angle angular sector width for symmetry
   */
  void initializeAngularSymmetry(const Point & axis, const Real & angle);

  /**
   * Whether point is on the positive side of a plane; points exactly on plane return false
   * @param[in] p point
   * @param[in] normal unit normal defining the plane
   * @return whether on positive side of plane
   */
  bool onPositiveSideOfPlane(const Point & p, const Point & normal) const;

  /**
   * Reflect point across a plane
   * @param[in] p point
   * @param[in] normal unit normal defining the plane
   * @return reflected point
   */
  Point reflectPointAcrossPlane(const Point & p, const Point & normal) const;

  /**
   * Rotate point about an axis
   * @param[in] p point
   * @param[in] angle angle to rotate (radians)
   * @param[in] axis axis expressed as vector
   * @return rotated point
   */
  Point rotatePointAboutAxis(const Point & p, const Real & angle, const Point & axis) const;

  /**
   * Transform point coordinates according to class settings
   * @param[in] p point
   * @return transformed point
   */
  Point transformPoint(const Point & p) const;

  /**
   * Sector of point
   * @param[in] p point
   * @return sector
   */
  int sector(const Point & p) const;

protected:
  /// Normal defining the first symmetry plane
  Point _normal;

  /// Axis of angular rotation symmetry
  Point _rotational_axis;

  /// Rotation angle
  Real _angle;

  /// Line defining the "zero-theta" line for rotational symmetry
  Point _zero_theta;

  /// Normal defining the reflection plane, for odd-numbered sectors
  Point _reflection_normal;

  /// Whether rotational symmetry is applied
  bool _rotational_symmetry;
};
