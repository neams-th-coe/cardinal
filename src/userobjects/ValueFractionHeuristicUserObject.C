#include "ValueFractionHeuristicUserObject.h"

registerMooseObject("CardinalApp", ValueFractionHeuristicUserObject);

InputParameters
ValueFractionHeuristicUserObject::validParams()
{

  InputParameters param = ClusteringUserObjectBase::validParams();
  param.addRequiredRangeCheckedParam<Real>("upper_fraction",
                                           "0.0 < upper_fraction &  upper_fraction < 1.0",
                                           "upper percentage of error for the heuristics");
  param.addRequiredRangeCheckedParam<Real>("lower_fraction",
                                           "0.0 < lower_fraction & lower_fraction < 1.0 ",
                                           "lower percentage of error for the heuristics");
  param.addClassDescription("Clusters elements whose variable values both fall within either a"
                            "specified upper or lower fraction of the extremes.");

  return param;
}
ValueFractionHeuristicUserObject::ValueFractionHeuristicUserObject(const InputParameters & params)
  : ClusteringUserObjectBase(params),
    _upper_fraction(getParam<Real>("upper_fraction")),
    _lower_fraction(getParam<Real>("lower_fraction")),
    _max(std::numeric_limits<Real>::min()),
    _min(std::numeric_limits<Real>::max())
{
  if (_lower_fraction + _upper_fraction > 1)
    mooseError("lower_fraction + upper_fraction must be less than 1");
}

void
ValueFractionHeuristicUserObject::extremesFinder()
{

  for (auto & elem : _mesh.active_element_ptr_range())
  {
    auto score = getMetricData(elem);
    _max = std::max(_max, score);
    _min = std::min(_min, score);
  }
  _upper_cut_off = (1 - _upper_fraction) * (_max - _min) + _min;
  _lower_cut_off = _lower_fraction * (_max - _min) + _min;
}

bool
ValueFractionHeuristicUserObject::evaluate(libMesh::Elem * base_element,
                                           libMesh::Elem * neighbor_element) const
{

  Real base_score = getMetricData(base_element);
  Real neighbor_score = getMetricData(neighbor_element);
  return (base_score > _upper_cut_off && neighbor_score > _upper_cut_off) ||
         (base_score < _lower_cut_off && neighbor_score < _lower_cut_off);
}
