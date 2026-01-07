#include "ThresholdHeuristicUserObject.h"

registerMooseObject("CardinalApp", ThresholdHeuristicUserObject);

InputParameters
ThresholdHeuristicUserObject::validParams()
{

  InputParameters params = ClusteringHeuristicUserObjectBase::validParams();
  params.addRequiredParam<double>("threshold",
                                  " The value against which the clustering process is compared.");
  params.addParam<bool>("cluster_if_above_threshold",
                        true,
                        " Cluster elements if the value is more than the threshold");
  params.addClassDescription(
      "Clusters elements whose scores are less/more than a specified threshold.");

  return params;
}

ThresholdHeuristicUserObject::ThresholdHeuristicUserObject(const InputParameters & parameters)
  : ClusteringHeuristicUserObjectBase(parameters),
    _threshold(getParam<double>("threshold")),
    _cluster_if_above_threshold(getParam<bool>("cluster_if_above_threshold"))
{
}

bool
ThresholdHeuristicUserObject::evaluate(libMesh::Elem * elem, libMesh::Elem * neighbor_elem) const
{

  return _cluster_if_above_threshold
             ? ((getMetricData(elem) > _threshold && getMetricData(neighbor_elem) > _threshold))
             : ((getMetricData(elem) < _threshold && getMetricData(neighbor_elem) < _threshold));
}
