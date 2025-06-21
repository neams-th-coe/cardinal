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

#include "ErrorFractionLookAheadMarker.h"

#include "libmesh/error_vector.h"

registerMooseObject("MooseApp", ErrorFractionLookAheadMarker);

InputParameters
ErrorFractionLookAheadMarker::validParams()
{
  InputParameters params = ErrorFractionMarker::validParams();
  params.addClassDescription(
      "Marks elements for refinement or coarsening based on the fraction of "
      "the min/max error from the supplied indicator and the relative error of a tally value.");
  params.addRequiredRangeCheckedParam<Real>(
      "rel_error_refine", "0 <= rel_error_refine <= 1", "The relative error refinement threshold.");
  params.addRequiredParam<IndicatorName>(
      "stat_error_indicator",
      "The name of the statistical relative error Indicator that this Marker uses.");

  return params;
}

ErrorFractionLookAheadMarker::ErrorFractionLookAheadMarker(const InputParameters & parameters)
  : ErrorFractionMarker(parameters),
    _rel_error_limit(getParam<Real>("rel_error_refine")),
    _rel_error_vec(getErrorVector(parameters.get<IndicatorName>("stat_error_indicator")))
{
}

void
ErrorFractionLookAheadMarker::markerSetup()
{
  if (_clear_extremes)
  {
    _min = std::numeric_limits<Real>::max();
    _max = 0;
  }

  // First find the max and min error
  for (unsigned int i = 0; i < _rel_error_vec.size(); ++i)
  {
    // Initial pruning step: elements only contribute to the min/max spatial
    // error if their relative errors are sufficiently low.
    if (_rel_error_vec[i] < _rel_error_limit)
    {
      _min = std::min(_min, static_cast<Real>(_error_vector[i]));
      _max = std::max(_max, static_cast<Real>(_error_vector[i]));
    }
  }

  _delta = _max - _min;
  _refine_cutoff = (1.0 - _refine) * _max;
  _coarsen_cutoff = _coarsen * _delta + _min;
}

Marker::MarkerValue
ErrorFractionLookAheadMarker::computeElementMarker()
{
  // Lookahead statistical error in an element.
  Real m = std::sqrt(_current_elem->n_children());
  Real stat_error = _rel_error_vec[_current_elem->id()];

  // Spatial error in an element.
  Real error = _error_vector[_current_elem->id()];

  if (error > _refine_cutoff && (stat_error * m) <= _rel_error_limit)
    return REFINE;
  else if (error < _coarsen_cutoff || stat_error > _rel_error_limit)
    return COARSEN;

  // Goldilocks zone, do nothing.
  return DO_NOTHING;
}
