#pragma once

#include "ReynoldsNumber.h"

class PecletNumber;

template <>
InputParameters validParams<PecletNumber>();

/**
 * Compute the Peclet number as
 * \f$Re\ Pr\f$, where $Re$ is the Reynolds number and \f$Pr\f$ is the
 * Prandtl number. The 'boundary' is used to specify which
 * boundary of NekRS's mesh to use for computing the mass flowrate and area.
 *
 * If NekRS is run in dimensional form, then the characteristic length
 * must be provided manually - otherwise, it is inferred from the
 * Reynolds number given in the input file.
 */
class PecletNumber : public ReynoldsNumber
{
public:
  static InputParameters validParams();

  PecletNumber(const InputParameters & parameters);

  virtual Real getValue() override;
};
