#pragma once

#include "NekSideFieldPostprocessor.h"

class NekSideIntegral;

template <>
InputParameters validParams<NekSideIntegral>();

/**
 * Compute the integral of a specified quantity over the boundaries
 * in the nekRS mesh, \f$\int_\Gamma f d\Gamma\f$, where \f$f\f$ is the
 * field to be integrated, and \f$\Gamma\f$ is the boundary.
 *
 * Note that this calculation is done directly on the mesh that nekRS solves on,
 * _not_ the mesh created for solution transfer in NekRSMesh.
 */
class NekSideIntegral : public NekSideFieldPostprocessor
{
public:
  static InputParameters validParams();

  NekSideIntegral(const InputParameters & parameters);

  virtual Real getValue() override;
};

