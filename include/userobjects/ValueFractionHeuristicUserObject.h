#pragma once

#include "ClusteringHeuristicUserObjectBase.h"

/*A clustering heuristic user object which clusters two neighboring elements whose scores
 * are either more/less than a percentage of extremes.
 * */

class ValueFractionHeuristicUserObject : public ClusteringHeuristicUserObjectBase
{

public:
  static InputParameters validParams();
  ValueFractionHeuristicUserObject(const InputParameters & params);

  /// method for evaluting if element should be clusterd or not
  virtual bool evaluate(libMesh::Elem * base_element, libMesh::Elem * neighbor_elem) const override;

protected:
  /// maximum and minmum value finder
  void extremesFinder();

  virtual void execute() override { extremesFinder(); };

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
