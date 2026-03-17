#pragma once

#include "ClusteringUserObjectBase.h"

/**
 * A clustering heuristic user object which clusters two neighboring elements whose metric variable
 * values are within a user provided range
 */
class ValueRangeHeuristicUserObject : public ClusteringUserObjectBase
{

public:
  static InputParameters validParams();
  ValueRangeHeuristicUserObject(const InputParameters & params);

  virtual bool evaluate(libMesh::Elem * base_element, libMesh::Elem * neighbor_elem) const override;

protected:
  /// tolerance percentage for calculating the lower and upper limit
  const Real & _tolerance_percentage;

  /// user provided value
  const Real & _value;

  /// upper limit of the range
  const Real & _upper_limit;

  /// lower limit of the range
  const Real & _lower_limit;

private:
  /**
   * Method for determining  if the metric variable value is within the range (_lower_limit,
   * _upper_limit)
   * @param[in] element a libmesh element
   * @return decides if the variable value is within a range
   */
  bool isInsideTheRange(libMesh::Elem * element) const;
};
