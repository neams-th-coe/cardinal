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

#include "ErrorFractionLookAheadMarker.h"

#include "libmesh/error_vector.h"

registerMooseObject("MooseApp", ErrorFractionLookAheadMarker);

InputParameters
ErrorFractionLookAheadMarker::validParams()
{
  InputParameters params = ErrorFractionMarker::validParams();
  params.addClassDescription("Marks elements for refinement or coarsening based on the fraction of "
                             "the min/max error from the supplied indicator and the relative error of a tally value.");
  params.addRequiredRangeCheckedParam<Real>("rel_error_refine", "0 <= rel_error_refine <= 1", "The relative error refinement threshold.");
  params.addRequiredParam<MooseEnum>("tally_score",
                                     getSingleTallyScoreEnum(),
                                     "The tally score used for the relative error.");

  return params;
}

ErrorFractionLookAheadMarker::ErrorFractionLookAheadMarker(const InputParameters & parameters)
  : ErrorFractionMarker(parameters),
    OpenMCBase(this, parameters),
    _rel_error_limit(getParam<Real>("rel_error_refine"))
{
  std::string score = getParam<MooseEnum>("tally_score");
  std::replace(score.begin(), score.end(), '_', '-');

  if (!_openmc_problem->hasScore(score))
    paramError("tally_score",
               "The problem does not contain any score named " +
               std::string(getParam<MooseEnum>("tally_score")) +
               "! Please "
               "ensure that one of your [Tallies] is scoring the requested reaction rate.");

  if (!_openmc_problem->hasOutput(score, "rel_error"))
    mooseError("The problem does not contain any tallies that output the relative error for the score "
               + std::string(getParam<MooseEnum>("tally_score")) + "!");

  // Check to ensure the reaction rate / flux variables are CONSTANT MONOMIALS.
  bool const_mon = true;
  for (const auto v : _openmc_problem->getTallyScoreVariables(score, _tid, "_rel_error"))
    const_mon &= v->feType() == FEType(CONSTANT, MONOMIAL);

  if (!const_mon)
    paramError("tally_score",
               "ElementOpticalDepthIndicator only supports CONSTANT MONOMIAL field variables. "
               "Please ensure your [Tallies] are adding CONSTANT MONOMIAL field variables.");

  // Grab the relative error from the [Tallies].
  _tally_rel_error = _openmc_problem->getTallyScoreVariableValues(score, _tid, "_rel_error");
}

Marker::MarkerValue
ErrorFractionLookAheadMarker::computeElementMarker()
{
  Real max_rel_error = 0.0;
  for (const auto & var : _tally_rel_error)
    max_rel_error = std::max((*var)[0], max_rel_error);
  max_rel_error *= _current_elem->n_children();

  Real error = _error_vector[_current_elem->id()];

  if (error > _refine_cutoff && max_rel_error <= _rel_error_limit)
    return REFINE;
  else if (error < _coarsen_cutoff && max_rel_error > _rel_error_limit)
    return COARSEN;

  return DO_NOTHING;
}
