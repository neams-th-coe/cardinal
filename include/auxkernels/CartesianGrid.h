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

#include "AuxKernel.h"

/**
 * Subdivide space into 3-D Cartesian grid; this can be used in
 * conjunction with skinning to change the topology of DAGMC geometries,
 * for instance.
 */
class CartesianGrid : public AuxKernel
{
public:
  CartesianGrid(const InputParameters & parameters);

  static InputParameters validParams();

protected:
  virtual Real computeValue();

  /// Number of divisions along x direction
  const unsigned int & _nx;

  /// Number of divisions along y direction
  const unsigned int & _ny;

  /// Number of divisions along z direction
  const unsigned int & _nz;

  /// Width of x regions
  Real _dx;

  /// Width of y regions
  Real _dy;

  /// Width of z regions
  Real _dz;

  /// Lower left of bounding box over mesh
  Point _min;
};
