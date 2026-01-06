#pragma once

#include "ClusteringHeuristicUserObjectBase.h"

/**
 * A clustering heuristic user object which clusters two neighboring elements whose scores
 * are either more/less than a percentage of extremes.
 */
class ValueFractionHeuristicUserObject : public ClusteringHeuristicUserObjectBase
{

public:
  static InputParameters validParams();
  ValueFractionHeuristicUserObject(const InputParameters & params);

  virtual bool evaluate(libMesh::Elem * base_element, libMesh::Elem * neighbor_elem) const override;

  virtual void execute() override { extremesFinder(); };

protected:
  /// maximum and minimum value finder
  void extremesFinder();

  /// upper fraction of the metric value
  Real _upper_fraction;

  /// lower fraction of the metric value
  Real _lower_fraction;

  /// maximum value of the metric
  Real _max;

  /// minimum value of the metric
  Real _min;

  /// upper cut off of the metric value
  Real _upper_cut_off;

  /// upper cut off of the metric value
  Real _lower_cut_off;
};
