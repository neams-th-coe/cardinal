#include "ClusteringHeuristicUserObjectBase.h"
#include "AuxiliarySystem.h"

InputParameters
ClusteringHeuristicUserObjectBase::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params.addRequiredParam<AuxVariableName>(
      "metric_variable_name", "The name of the variable based on which clustering will be done");

  return params;
}

ClusteringHeuristicUserObjectBase::ClusteringHeuristicUserObjectBase(const InputParameters & parameters)
  : GeneralUserObject(parameters),
    _mesh(_fe_problem.mesh().getMesh()),
    _metric_variable_name(getParam<AuxVariableName>("metric_variable_name")),
    _metric_variable(_fe_problem.getVariable(_tid, _metric_variable_name)),
    _auxiliary_system(_fe_problem.getAuxiliarySystem()),
    _dof_map(_auxiliary_system.dofMap()),
    _metric_variable_index(_auxiliary_system.getVariable(_tid, _metric_variable_name).number())
{
  //check if the element type if CONSTANT MONOMIAL. If not then throw a mooseError.
  if (_metric_variable.feType()!=FEType(CONSTANT, MONOMIAL))
    mooseError("Variable must be type of CONSTANT MONOMIAL");
}


Real
ClusteringHeuristicUserObjectBase::getMetricData(const libMesh::Elem * elem) const
{

  std::vector<libMesh::dof_id_type> dof_indices;
  std::vector<double> solution_value(1);
  _dof_map.dof_indices(elem, dof_indices, _metric_variable_index);
  _auxiliary_system.solution().get(dof_indices, solution_value);

  return solution_value[0];
}