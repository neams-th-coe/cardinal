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

#include "BooleanComboMarker.h"

registerMooseObject("MooseApp", BooleanComboMarker);

InputParameters
BooleanComboMarker::validParams()
{
  auto params = Marker::validParams();
  params.addClassDescription("A class which takes multiple markers and ANDs / ORs them together to "
                             "determine if an element should be refined or coarsened.");
  params.addRequiredParam<std::vector<MarkerName>>(
      "refine_markers", "A list of the marker names to be used for refinement.");
  params.addRequiredParam<std::vector<MarkerName>>(
      "coarsen_markers", "A list of the marker names to be used for coasening.");
  params.addParam<MooseEnum>(
      "boolean_operator",
      MooseEnum("and or", "and"),
      "How the different markers should be combined. Options are to AND each "
      "marker together or to OR them together.");
  params.addParam<MooseEnum>("priority",
                             MooseEnum("refinement coarsening", "refinement"),
                             "The operator to be prioritized when marking elements.");

  return params;
}

BooleanComboMarker::BooleanComboMarker(const InputParameters & parameters)
  : Marker(parameters),
    _refine_marker_names(parameters.get<std::vector<MarkerName>>("refine_markers")),
    _coarsen_marker_names(parameters.get<std::vector<MarkerName>>("coarsen_markers")),
    _use_and(getParam<MooseEnum>("boolean_operator") == "and"),
    _priority_refinement(getParam<MooseEnum>("priority") == "refinement")
{
  for (const auto & marker_name : _refine_marker_names)
    _refine_markers.push_back(&getMarkerValue(marker_name));

  for (const auto & marker_name : _coarsen_marker_names)
    _coarsen_markers.push_back(&getMarkerValue(marker_name));
}

Marker::MarkerValue
BooleanComboMarker::computeElementMarker()
{
  bool refine = _use_and;
  bool coarsen = _use_and;

  for (const auto m : _refine_markers)
  {
    if (_use_and)
      refine &= static_cast<MarkerValue>((*m)[0]) == MarkerValue::REFINE;
    else
      refine |= static_cast<MarkerValue>((*m)[0]) == MarkerValue::REFINE;
  }

  for (const auto m : _coarsen_markers)
  {
    if (_use_and)
      coarsen &= static_cast<MarkerValue>((*m)[0]) == MarkerValue::COARSEN;
    else
      coarsen |= static_cast<MarkerValue>((*m)[0]) == MarkerValue::COARSEN;
  }

  if (_priority_refinement)
  {
    if (refine)
      return MarkerValue::REFINE;
    if (coarsen)
      return MarkerValue::COARSEN;
  }
  else
  {
    if (coarsen)
      return MarkerValue::COARSEN;
    if (refine)
      return MarkerValue::REFINE;
  }

  return MarkerValue::DO_NOTHING;
}
