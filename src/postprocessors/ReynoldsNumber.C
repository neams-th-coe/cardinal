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

#ifdef ENABLE_NEK_COUPLING

#include "ReynoldsNumber.h"
#include "NekInterface.h"
#include "UserErrorChecking.h"

registerMooseObject("CardinalApp", ReynoldsNumber);

InputParameters
ReynoldsNumber::validParams()
{
  InputParameters params = NekSidePostprocessor::validParams();
  params.addRangeCheckedParam<Real>("L_ref", "L_ref > 0.0", "Reference length scale");
  params.addClassDescription("Compute the Reynolds number characteristic of the NekRS solution");
  return params;
}

ReynoldsNumber::ReynoldsNumber(const InputParameters & parameters)
  : NekSidePostprocessor(parameters)
{
  // for dimensional cases, we need to provide the characteristic length ourselves
  // because there's no way to infer it
  if (!_nek_problem->nondimensional())
  {
    checkRequiredParam(parameters, "L_ref", "running NekRS in dimensional form");
    _L_ref = &getParam<Real>("L_ref");
  }
  else
    checkUnusedParam(parameters, "L_ref", "running NekRS in non-dimensional form");
}

Real
ReynoldsNumber::getValue() const
{
  Real area = nekrs::area(_boundary, _pp_mesh);
  Real mdot = std::abs(nekrs::sideMassFluxWeightedIntegral(_boundary, field::unity, _pp_mesh));
  Real mu = nekrs::viscosity();
  Real L = _nek_problem->nondimensional() ? _nek_problem->L_ref() : *_L_ref;

  return mdot * L / (area * mu);
}

#endif
