#include "ValueRangeHeuristicUserObject.h"

registerMooseObject("CardinalApp", ValueRangeHeuristicUserObject);

InputParameters
ValueRangeHeuristicUserObject::validParams()
{

  InputParameters params = ClusteringUserObjectBase::validParams();
  params.addRequiredRangeCheckedParam<Real>("tolerance_percentage",
                                            "tolerance_percentage>0 & tolerance_percentage<1",
                                            "Fractional tolerance used to define"
                                            " the acceptable range around the reference value.");
  params.addRequiredParam<Real>(
      "value", "The reference value to decide if an element is close enough to be clustered");
  params.addClassDescription(
      "Clusters elements if their value is within the tolerance of the value provided.");

  return params;
}

ValueRangeHeuristicUserObject::ValueRangeHeuristicUserObject(const InputParameters & params)
  : ClusteringUserObjectBase(params),
    _tolerance_percentage(getParam<Real>("tolerance_percentage")),
    _value(getParam<Real>("value")),
    _upper_limit((1 + _tolerance_percentage) * _value),
    _lower_limit((1 - _tolerance_percentage) * _value)
{
}

bool
ValueRangeHeuristicUserObject::isInsideTheRange(libMesh::Elem * element) const
{
  Real metric_value = getMetricData(element);
  return _lower_limit < metric_value && metric_value < _upper_limit;
}

bool
ValueRangeHeuristicUserObject::evaluate(libMesh::Elem * base_element,
                                        libMesh::Elem * neighbor_element) const
{
  return isInsideTheRange(base_element) && isInsideTheRange(neighbor_element);
}
