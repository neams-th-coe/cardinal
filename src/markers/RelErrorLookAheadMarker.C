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

#include "RelErrorLookAheadMarker.h"

registerMooseObject("MooseApp", RelErrorLookAheadMarker);

InputParameters
RelErrorLookAheadMarker::validParams()
{
  auto params = QuadraturePointMarker::validParams();
  params.addClassDescription(
    "A class which looks ahead at the relative error if a tally field. "
    "If the estimated tally field after the element is split is less than "
    "a user specified threshold, then proceed with refinement.");
  params.addRequiredRangeCheckedParam<Real>("refine", "0 <= refine <= 1", "The refinement threshold.");

  return params;
}

RelErrorLookAheadMarker::RelErrorLookAheadMarker(const InputParameters & parameters)
  : QuadraturePointMarker(parameters)
  , _rel_error_limit(getParam<Real>("refine"))
{ }

Marker::MarkerValue
RelErrorLookAheadMarker::computeQpMarker()
{
  auto rel_e_after_split = _u[_qp] * _current_elem->n_children();
  if (rel_e_after_split <= _rel_error_limit)
    return Marker::MarkerValue::REFINE;
  else
    return Marker::MarkerValue::DO_NOTHING;
}
