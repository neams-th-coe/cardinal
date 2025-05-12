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

#ifdef ENABLE_OPENMC_COUPLING

#include "ComputeMGXSAux.h"

registerMooseObject("CardinalApp", ComputeMGXSAux);

InputParameters
ComputeMGXSAux::validParams()
{
  auto params = OpenMCAuxKernel::validParams();
  params.addClassDescription(
      "An auxkernel that computes a multi-group cross section using a list of group-wise reaction "
      "rates and a list of "
      "normalization factors. This is intended to be added by the MGXS action.");
  params.addRequiredCoupledVar(
      "rxn_rates",
      "The group-wise reaction rates to use for computing the multi-group cross section.");
  params.addRequiredCoupledVar("normalize_by",
                               "The normalization factor to use when computing multi-group cross "
                               "sections. This is usually the group-wise scalar flux.");

  return params;
}

ComputeMGXSAux::ComputeMGXSAux(const InputParameters & parameters) : OpenMCAuxKernel(parameters)
{
  for (unsigned int i = 0; i < coupledComponents("rxn_rates"); ++i)
    _mg_reaction_rates.emplace_back(&coupledValue("rxn_rates", i));

  for (unsigned int i = 0; i < coupledComponents("normalize_by"); ++i)
    _norm_factors.emplace_back(&coupledValue("normalize_by", i));
}

Real
ComputeMGXSAux::computeValue()
{
  Real mgxs = 0.0;
  Real norm = 0.0;
  for (unsigned int i = 0; i < _mg_reaction_rates.size(); ++i)
    mgxs += (*_mg_reaction_rates[i])[_qp];
  for (unsigned int i = 0; i < _norm_factors.size(); ++i)
    norm += (*_norm_factors[i])[_qp];

  return norm > 0.0 ? mgxs / norm : 0.0;
}

#endif
