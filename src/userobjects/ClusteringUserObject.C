#include "ClusteringUserObject.h"
#include "AuxiliarySystem.h"

InputParameters
ClusteringUserObject::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params.addRequiredParam<ExtraElementIDName>("id_name", "extra_element_integer_id name");
  params.addRequiredParam<AuxVariableName>(
      "metric_variable_name", "The name of the variable based on which clustering will be done");
  params.addClassDescription("Base clustering object that for other heuristic based user obejct. ");

  return params;
}

ClusteringUserObject::ClusteringUserObject(const InputParameters & parameters)
  : GeneralUserObject(parameters),
    _id_name(getParam<ExtraElementIDName>("id_name")),
    _mesh(_fe_problem.mesh().getMesh()),
    _metric_variable_name(getParam<AuxVariableName>("metric_variable_name")),
    _metric_variable(_fe_problem.getVariable(_tid, _metric_variable_name)),
    _auxiliary_system(_fe_problem.getAuxiliarySystem()),
    _dof_map(_auxiliary_system.dofMap()),
    _metric_variable_index(_auxiliary_system.getVariable(_tid, _metric_variable_name).number())
{
  if (!_mesh.has_elem_integer(_id_name))
  {
    mooseWarning("Mesh does not have an extra element integer named ",
                 _id_name,
                 "."
                 " so adding the ",
                 _id_name,
                 " generator defines it with extra_element_integers.");
    _mesh.add_elem_integer(_id_name);
  }
  if (_metric_variable.feType()!=FEType(CONSTANT, MONOMIAL))
    mooseError("Variable must be type of CONSTANT MONOMIAL");
  _extra_integer_index = _mesh.get_elem_integer_index(_id_name);
}

void
ClusteringUserObject::execute()
{
  applyNoClusteringInitialCondition();
  findCluster();
}

Real
ClusteringUserObject::getMetricData(const libMesh::Elem * elem)
{

  std::vector<libMesh::dof_id_type> dof_indices;
  std::vector<double> solution_value(1);
  _dof_map.dof_indices(elem, dof_indices, _metric_variable_index);
  _auxiliary_system.solution().get(dof_indices, solution_value);

  return static_cast<Real>(solution_value[0]);
}

void
ClusteringUserObject::applyNoClusteringInitialCondition()
{
  for (auto & elem : _mesh.active_element_ptr_range())
  {
    elem->set_extra_integer(_extra_integer_index, not_visited);
  }
}

void
ClusteringUserObject::findCluster()
{

  std::stack<libMesh::Elem *> neighbor_stack;

  for (auto & elem : _mesh.active_element_ptr_range())
  {
    if (elem->get_extra_integer(_extra_integer_index) != not_visited)
    {
      continue;
    }
    int cluster_id = elem->id();
    neighbor_stack.push(elem);

    while (!neighbor_stack.empty())
    {
      libMesh::Elem * current_elem = neighbor_stack.top();
      neighbor_stack.pop();

      for (unsigned int s = 0; s < current_elem->n_sides(); s++)
      {
        libMesh::Elem * neighbor_elem = current_elem->neighbor_ptr(s);

        if (neighbor_elem && neighbor_elem->active() &&
            neighbor_elem->get_extra_integer(_extra_integer_index) == not_visited)
        {
          if (belongsToCluster(current_elem, neighbor_elem))
          {

            elem->set_extra_integer(_extra_integer_index, cluster_id);
            neighbor_elem->set_extra_integer(_extra_integer_index, cluster_id);
            neighbor_stack.push(neighbor_elem);
          }
        }
      }
    }
  }
}

int
ClusteringUserObject::getExtraIntegerScore(libMesh::Elem * elem)
{
  return elem->get_extra_integer(_extra_integer_index);
}