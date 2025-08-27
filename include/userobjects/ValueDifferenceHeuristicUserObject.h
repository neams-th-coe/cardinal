#pragma once

#include "ClusteringHeuristicUserObjectBase.h"

/*A clustering heuristic user object which clusters elements whose relative score
 * differences are less than the specified tolerance.
 * */
class ValueDifferenceHeuristicUserObject : public ClusteringHeuristicUserObjectBase
{

public:
  static InputParameters validParams();
  ValueDifferenceHeuristicUserObject(const InputParameters & params);

  /// method for evaluting if element should be clusterd or not
  virtual bool evaluate(libMesh::Elem * base_element, libMesh::Elem * neighbor_elem) const override;
protected:

  /// relative tolerance
  const Real _tolerance;
};