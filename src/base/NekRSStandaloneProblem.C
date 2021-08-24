#include "NekRSStandaloneProblem.h"
#include "NekInterface.h"

registerMooseObject("CardinalApp", NekRSStandaloneProblem);

template<>
InputParameters
validParams<NekRSStandaloneProblem>()
{
  InputParameters params = validParams<NekRSProblemBase>();
  return params;
}

NekRSStandaloneProblem::NekRSStandaloneProblem(const InputParameters &params) : NekRSProblemBase(params)
{
  // It doesn't make sense to specify both a boundary and volume for a
  // standalone case, because we are only using the boundary/volume for
  // data extraction (and if you set 'volume = true', you automatically get
  // the solution on the boundaries as well).
  if (_nek_mesh->boundary() && _nek_mesh->volume())
    mooseWarning("When 'volume = true' for '" + type() + "', it is redundant to also set 'boundary'.\n"
      "Boundary IDs will be ignored.");
}

bool
NekRSStandaloneProblem::movingMesh() const
{
  return nekrs::hasMovingMesh();
}
