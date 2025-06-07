#pragma once

#include "ClusteringUserObject.h"

class ValueDifferenceHeuristicUserObject : public ClusteringUserObject
{

public:
  static InputParameters validParams();
  ValueDifferenceHeuristicUserObject(const InputParameters & params);
  virtual bool belongsToCluster(libMesh::Elem * base_element,
                                libMesh::Elem * neighbor_elem) override;

protected:
  Real _tolerance;
};