#pragma once

#include "ClusteringHeuristicUserObjectBase.h"

/*A clustering heuristic user object which clusters two neighboring elements whose scores
 * are more than a threshold.
 * */
class ThresholdHeuristicUserObject : public ClusteringHeuristicUserObjectBase
{

public:
  static InputParameters validParams();
  ThresholdHeuristicUserObject(const InputParameters & parameters);

  /// method for evaluting if element should be clusterd or not
  virtual bool evaluate(libMesh::Elem * elem, libMesh::Elem * neighbor_elem) const override;

private:
  // threshold limit for clustering
  const double _threshold;

  /// determines if we should cluster whether above of below the threshold limit
  bool _cluster_if_above_threshold;
};
