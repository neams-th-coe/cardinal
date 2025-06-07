#pragma once

#include "ClusteringUserObject.h"

class ValueRangeHeuristicUserObject : public ClusteringUserObject
{

public:
  static InputParameters validParams();
  ValueRangeHeuristicUserObject(const InputParameters & params);
  virtual bool belongsToCluster(libMesh::Elem * base_element,
                                libMesh::Elem * neighbor_elem) override;

protected:
  Real _tolerance_percentage;
  Real _value;
  Real _upper_limit;
  Real _lower_limit;

private:
  bool isInsideTheRange(libMesh::Elem * element);
};