#pragma once

#include "ClusteringUserObject.h"

class ThresholdHeuristicsUserObject : public ClusteringUserObject
{

public:
  static InputParameters validParams();
  ThresholdHeuristicsUserObject(const InputParameters & parameters);

protected:
  virtual bool belongsToCluster(libMesh::Elem * elem, libMesh::Elem * neighbor_elem) override;

private:
  double _threshold;
  bool _cluster_if_above_threshold;
};