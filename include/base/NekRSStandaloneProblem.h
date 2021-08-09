#pragma once

#include "NekRSProblemBase.h"

class NekRSStandaloneProblem;

template<>
InputParameters validParams<NekRSStandaloneProblem>();

/**
 * Run NekRS as a standalone application, without any data transfers
 * to/from MOOSE.
 */
class NekRSStandaloneProblem : public NekRSProblemBase
{
public:
  NekRSStandaloneProblem(const InputParameters & params);

  virtual bool movingMesh() const override;
};
