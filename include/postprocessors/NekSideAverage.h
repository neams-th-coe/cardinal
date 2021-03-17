//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "NekSideIntegral.h"

class NekSideAverage;

template <>
InputParameters validParams<NekSideAverage>();

/**
 * Compute the average of a specified quantity over the boundaries
 * in the nekRS mesh, \f$\frac{\int_\Gamma f d\Gamma}{\int_\Gamma d\Gamma}\f$,
 * where \f$f\f$ is the field to be integrated, and \f$\Gamma\f$ is the boundary.
 *
 * Note that this calculation is done directly on the mesh that nekRS solves on,
 * _not_ the mesh created for solution transfer in NekRSMesh.
 */
class NekSideAverage : public NekSideIntegral
{
public:
  static InputParameters validParams();

  NekSideAverage(const InputParameters & parameters);

  virtual Real getValue() override;

protected:
  /// Area by which to normalize
  Real _area;
};
