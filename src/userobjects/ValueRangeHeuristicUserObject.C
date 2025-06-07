#include "ValueRangeHeuristicUserObject.h"

registerMooseObject("CardinalApp", ValueRangeHeuristicUserObject);
InputParameters
ValueRangeHeuristicUserObject::validParams()
{

  InputParameters params = ClusteringUserObject::validParams();
  params.addRequiredParam<Real>("tolerance_percentage", "Fractional tolerance used to define"
                                                        " the acceptable range around the reference value."");
  params.addRequiredParam<Real>("value", "The reference value to decide if an element is close enough to be clustered");
  params.addClassDescription("Clusters elements if their score is within the tolerance of the value provided.");

  return params;
}

ValueRangeHeuristicUserObject::ValueRangeHeuristicUserObject(const InputParameters & params)
  : ClusteringUserObject(params),
    _tolerance_percentage(getParam<Real>("tolerance_percentage")),
    _value(getParam<Real>("value")),
    _upper_limit((1 + _tolerance_percentage) * _value),
    _lower_limit((1 - _tolerance_percentage) * _value)
{
}

bool
ValueRangeHeuristicUserObject::isInsideTheRange(libMesh::Elem * element)
{

  Real score = getMetricData(element);
  return _lower_limit < score and score < _upper_limit;
}
bool

ValueRangeHeuristicUserObject::belongsToCluster(libMesh::Elem * base_element,
                                                libMesh::Elem * neighbor_element)
{

  return isInsideTheRange(base_element) and isInsideTheRange(neighbor_element);
}