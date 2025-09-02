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

  /**
   * Method for determining if metric score
   * for two element whose scores are within a user provided range
   * param[in] base_element the current element
   * param[in] neighbor_elem the current neighbour of base_element
   * return whether the two elements should be added to a cluster or not
   */
  virtual bool evaluate(libMesh::Elem * base_element, libMesh::Elem * neighbor_elem) const override;

protected:
  Real _tolerance_percentage;
  Real _value;
  Real _upper_limit;
  Real _lower_limit;

private:
  /**
   * Method for determining  if the score is with the range (_lower_limit, _upper_limit)
   * @param[in] an element
   * @param[out] if with in a range
   */
  bool isInsideTheRange(libMesh::Elem * element) const;
};
