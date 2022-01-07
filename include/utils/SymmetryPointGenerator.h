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
 * Class that rotates/reflects a point about symmetry planes
 */
class SymmetryPointGenerator
{
public:
  SymmetryPointGenerator(const Point & point, const Point & normal, const Real & angle);

  /**
   * Whether point is on the positive side of a plane; points exactly on plane return false
   * @param[in] p point
   * @return whether on positive side of plane
   */
  bool onPositiveSideOfPlane(const Point & p) const;

  Point reflectPointAcrossPlane(const Point & p) const;

protected:
  /// Point defining the plane
  const Point & _point;

  /// Normal defining the plane
  const Point & _normal;

  /// Angle of symmetry
  const Real _angle;

  /// Unit normal defining the plane
  Point _unit_normal;

  /// Number of symmetry sectors
  int _n_sectors;

  /// Coefficient defining plane, ax + by + cz = d
  Real _a;

  /// Coefficient defining plane, ax + by + cz = d
  Real _b;

  /// Coefficient defining plane, ax + by + cz = d
  Real _c;

  /// Coefficient defining plane, ax + by + cz = d
  Real _d;

  /// a*a + b*b + c*c, saved here for faster evaluations
  Real _denominator;
};
