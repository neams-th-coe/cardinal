#pragma once

#include "ClusteringHeuristicUserObjectBase.h"

class ThresholdHeuristicsUserObject : public ClusteringHeuristicUserObjectBase
{

public:
  static InputParameters validParams();
  ThresholdHeuristicsUserObject(const InputParameters & parameters);

protected:
  /// method for evaluting if element should be clusterd or not
  virtual bool evaluate(libMesh::Elem * elem, libMesh::Elem * neighbor_elem) const override;

private:
  //threshold limit for clustering
  const double _threshold;

  /// determines if we should cluster whether above of below the threshold limit
  bool _cluster_if_above_threshold;
};