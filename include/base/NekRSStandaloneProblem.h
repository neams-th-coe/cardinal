#pragma once

#include "NekRSProblemBase.h"

/**
 * Run NekRS as a standalone application, without any data transfers
 * to/from MOOSE.
 */
class NekRSStandaloneProblem : public NekRSProblemBase
{
public:
  NekRSStandaloneProblem(const InputParameters & params);

  static InputParameters validParams();

  virtual bool movingMesh() const override;
};
