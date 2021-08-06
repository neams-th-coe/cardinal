//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "NekSidePostprocessor.h"

class ReynoldsNumber;

template <>
InputParameters validParams<ReynoldsNumber>();

/**
 * Compute the Reynolds number as
 * \f$\frac{\dot{m}L}{A\mu}\f$, where $\dot{m}$ is the mass flowrate,
 * \f$L\f$ is a characteristic length, \f$A\f$ is the area, and \f$\mu\f$
 * is the dynamic viscosity. The 'boundary' is used to specify which
 * boundary of NekRS's mesh to use for computing the mass flowrate and area.
 *
 * If NekRS is run in dimensional form, then the characteristic length
 * must be provided manually - otherwise, it is inferred from the
 * Reynolds number given in the input file.
 */
class ReynoldsNumber : public NekSidePostprocessor
{
public:
  static InputParameters validParams();

  ReynoldsNumber(const InputParameters & parameters);

  virtual Real getValue() override;

protected:
  /// Characteristic length
  const Real * _L_ref;

  /// Area by which to compute the Reynolds number
  Real _area;
};
