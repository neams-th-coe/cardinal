#include "NekRSStandaloneProblem.h"

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
}
