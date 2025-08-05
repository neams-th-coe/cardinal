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

#include "ThreadedGeneralUserObject.h"

/**
 * Class that maps from a point (x, y, z) to a new point that is
 * either rotationally symmetry or mirrored. The origin for reflection
 * and rotation is (0, 0, 0).
 */
class SymmetryPointGenerator : public ThreadedGeneralUserObject
{
public:
  static InputParameters validParams();

  SymmetryPointGenerator(const InputParameters & parameters);

  virtual void initialize() {}
  virtual void finalize() {}
  virtual void execute() {}
  virtual void threadJoin(const UserObject & /* uo */) override {}

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
  /// Whether rotational symmetry is applied; otherwise, the domain is mirror-symmetric
  const bool _rotational_symmetry;

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
};
