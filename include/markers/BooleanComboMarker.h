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

#pragma once

#include "Marker.h"

/**
 * A class which takes multiple markers and ANDs / ORs
 * them together to determine if an element should be refined or coarsened.
 * By default refinement is prioritized over coarsening.
 */
class BooleanComboMarker : public Marker
{
public:
  static InputParameters validParams();

  BooleanComboMarker(const InputParameters & parameters);

protected:
  virtual MarkerValue computeElementMarker() override;

  /// The names of the markers to use for refinement.
  const std::vector<MarkerName> & _refine_marker_names;

  /// The names of the markers to use for coarsening.
  const std::vector<MarkerName> & _coarsen_marker_names;

  /// The marker variable values used for refinement.
  std::vector<const VariableValue *> _refine_markers;

  /// The marker variable values used for coarsening.
  std::vector<const VariableValue *> _coarsen_markers;

  /// Boolean flag to determine if an AND or OR should be used.
  const bool _use_and;

  /// Whether the priority is to refine elements or not.
  const bool _priority_refinement;
};
