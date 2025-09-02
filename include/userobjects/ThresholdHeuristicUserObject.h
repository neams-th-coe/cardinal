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

  /**
   * Method for determining if the metric score for two element is more/less than
   * a user defined threshold (_threshold) value.
   * param[in] base_element the current element
   * param[in] neighbor_elem the current neighbour of base_element
   * return whether the two elements should be added to a cluster or not
   */
  virtual bool evaluate(libMesh::Elem * base_element, libMesh::Elem * neighbor_elem) const override;

private:
  /// threshold limit for clustering
  const double _threshold;

  /// determines if we should cluster whether above of below the threshold limit
  bool _cluster_if_above_threshold;
};
