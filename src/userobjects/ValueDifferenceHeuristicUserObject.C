#include "ValueDifferenceHeuristicUserObject.h"

registerMooseObject("CardinalApp", ValueDifferenceHeuristicUserObject);

InputParameters
ValueDifferenceHeuristicUserObject::validParams()
{

  InputParameters params = ClusteringUserObject::validParams();
  params.addRequiredParam<Real>("tolerance", "Maximum allowed difference in scores for elements to be clustered");
  params.addClassDescription("Clusters elements whose score differences are less than the specified tolerance.");

  return params;
}

ValueDifferenceHeuristicUserObject::ValueDifferenceHeuristicUserObject(
    const InputParameters & params)
  : ClusteringUserObject(params), _tolerance(getParam<Real>("tolerance"))
{
}

bool
ValueDifferenceHeuristicUserObject::belongsToCluster(libMesh::Elem * base_element,
                                                     libMesh::Elem * neighbor_element)
{
  return std::abs(getMetricData(base_element) - getMetricData(neighbor_element)) < _tolerance;
}