#pragma once

#include "ClusteringHeuristicUserObjectBase.h"

/*A clustering heuristic user object which clusters two neighboring elements whose scores
 * are within a user provided range
 * */

class ValueRangeHeuristicUserObject : public ClusteringHeuristicUserObjectBase
{

public:
  static InputParameters validParams();
  ValueRangeHeuristicUserObject(const InputParameters & params);

  ///method for evaluting if element should be clusterd or not
  virtual bool evaluate(libMesh::Elem * base_element, libMesh::Elem * neighbor_elem) const override;

protected:
  Real _tolerance_percentage;
  Real _value;
  Real _upper_limit;
  Real _lower_limit;

private:
  ///determines if the score is with the range (_lower_limit, _upper_limit)
  bool isInsideTheRange(libMesh::Elem * element) const;
};
