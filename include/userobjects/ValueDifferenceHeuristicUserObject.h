#pragma once

#include "ClusteringUserObjectBase.h"

/**
 * A clustering heuristic user object which clusters elements whose relative metric variable value
 * differences are less than the specified tolerance.
 */
class ValueDifferenceHeuristicUserObject : public ClusteringUserObjectBase
{

public:
  static InputParameters validParams();
  ValueDifferenceHeuristicUserObject(const InputParameters & params);

  virtual bool evaluate(libMesh::Elem * base_element, libMesh::Elem * neighbor_elem) const override;

protected:
  /// relative tolerance
  const Real _tolerance;
};
