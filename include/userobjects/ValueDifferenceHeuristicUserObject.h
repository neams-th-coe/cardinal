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

  /**
   * Method for determining if the relative difference of the metric score
   * for two element is less than a tolerance value .
   * param[in] base_element the current element
   * param[in] neighbor_elem the current neighbour of base_element
   * return whether the two elements should be added to a cluster or not
   */
  virtual bool evaluate(libMesh::Elem * base_element, libMesh::Elem * neighbor_elem) const override;

protected:
  /// relative tolerance
  const Real _tolerance;
};
