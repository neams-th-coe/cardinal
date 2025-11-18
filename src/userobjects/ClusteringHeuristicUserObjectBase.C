#include "ClusteringHeuristicUserObjectBase.h"
#include "AuxiliarySystem.h"
#include "libmesh/dof_map.h"
#include "libmesh/mesh_base.h"
#include "libmesh/elem.h"

InputParameters
ClusteringHeuristicUserObjectBase::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params.addRequiredParam<AuxVariableName>(
      "metric_variable_name", "The name of the variable based on which clustering will be done");

  return params;
}

ClusteringHeuristicUserObjectBase::ClusteringHeuristicUserObjectBase(
    const InputParameters & parameters)
  : GeneralUserObject(parameters),
    _mesh(_fe_problem.mesh().getMesh()),
    _metric_variable_name(getParam<AuxVariableName>("metric_variable_name")),
    _metric_variable(_fe_problem.getVariable(_tid, _metric_variable_name)),
    _auxiliary_system(_fe_problem.getAuxiliarySystem()),
    _dof_map(_auxiliary_system.dofMap()),
    _metric_variable_index(_auxiliary_system.getVariable(_tid, _metric_variable_name).number()),
    _data_gathered(false)
{
  // check if the element type if CONSTANT MONOMIAL. If not then throw a mooseError.
  if (_metric_variable.feType() != FEType(CONSTANT, MONOMIAL))
    mooseError("Variable must be of type 'CONSTANT MONOMIAL'");
  // check if mesh is replicated. If not then throw a moose error.
  if (!_mesh.is_replicated())
    mooseError("Mesh must be replicated");
}

void
ClusteringHeuristicUserObjectBase::initialize()
{

  const auto & sol = _auxiliary_system.currentSolution();
  const dof_id_type n_dofs = sol->size();

  // create the serialized vector
  _serialized_metric_solution = libMesh::NumericVector<Real>::build(_communicator);
  _serialized_metric_solution->init(n_dofs, true, SERIAL);

  sol->localize(*_serialized_metric_solution);

  _data_gathered = true;
}

Real
ClusteringHeuristicUserObjectBase::getMetricData(const libMesh::Elem * elem) const
{
  std::vector<dof_id_type> dof_indices;
  _dof_map.dof_indices(elem, dof_indices, _metric_variable_index);
  if (_serialized_metric_solution->type() == SERIAL)
    return (*_serialized_metric_solution)(dof_indices[0]);
  else
    mooseError("Serialized solution vector not initialized as SERIAL!");
}
