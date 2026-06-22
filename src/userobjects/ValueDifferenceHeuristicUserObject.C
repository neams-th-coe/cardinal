#include "ValueDifferenceHeuristicUserObject.h"

registerMooseObject("CardinalApp", ValueDifferenceHeuristicUserObject);

InputParameters
ValueDifferenceHeuristicUserObject::validParams()
{

  InputParameters params = ClusteringUserObjectBase::validParams();
  params.addRequiredParam<Real>(
      "tolerance",
      "Maximum allowed absolute relative difference in values for elements to be clustered");
  params.addClassDescription(
      "Clusters elements whose relative value differences are less than the specified tolerance.");

  return params;
}

ValueDifferenceHeuristicUserObject::ValueDifferenceHeuristicUserObject(
    const InputParameters & params)
  : ClusteringUserObjectBase(params), _tolerance(getParam<Real>("tolerance"))
{
  if (_tolerance < 0.0)
    paramError("tolerance must be positive");
}

bool
ValueDifferenceHeuristicUserObject::evaluate(libMesh::Elem * base_element,
                                             libMesh::Elem * neighbor_element) const
{
  const Real base_metric_value = getMetricData(base_element);
  const Real neighbor_metric_value = getMetricData(neighbor_element);
  return std::abs((base_metric_value - neighbor_metric_value) / base_metric_value) < _tolerance;
}
