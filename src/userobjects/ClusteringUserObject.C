/********************************************************************/
/*                  SOFTWARE COPYRIGHT NOTIFICATION                 */
/*                             Cardinal                             */
/*                                                                  */
/*                  (c) 2021 UChicago Argonne, LLC                  */
/*                        ALL RIGHTS RESERVED                       */
/*                                                                  */
/*                 Prepared by UChicago Argonne, LLC                */
/*               Under Contract No. DE-AC02-06CH11357               */
/*                With the U. S. Department of Energy               */
/*                                                                  */
/*             Prepared by Battelle Energy Alliance, LLC            */
/*               Under Contract No. DE-AC07-05ID14517               */
/*                With the U. S. Department of Energy               */
/*                                                                  */
/*                 See LICENSE for full restrictions                */
/********************************************************************/

#include "ClusteringUserObject.h"
#include "AuxiliarySystem.h"

InputParameters
ClusteringUserObject::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params.addRequiredParam<ExtraElementIDName>("id_name", "extra_element_integer_id name");
  params.addRequiredParam<AuxVariableName>(
      "metric_variable_name", "The name of the variable based on which clustering will be done");

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
  //check if the element type if CONSTANT MONOMIAL. If not then throw a mooseError.
  if (_metric_variable.feType()!=FEType(CONSTANT, MONOMIAL))
    mooseError("Variable must be type of CONSTANT MONOMIAL");
  _extra_integer_index = _mesh.get_elem_integer_index(_id_name);
}

void
ClusteringUserObject::execute()
{
  //set the extra element integer value to NOT_VISITED
  applyNoClusteringInitialCondition();
  findCluster();
}

Real
ClusteringUserObject::getMetricData(const libMesh::Elem * elem) const
{

  std::vector<libMesh::dof_id_type> dof_indices;
  std::vector<double> solution_value(1);
  _dof_map.dof_indices(elem, dof_indices, _metric_variable_index);
  _auxiliary_system.solution().get(dof_indices, solution_value);

  return solution_value[0];
}

void
ClusteringUserObject::applyNoClusteringInitialCondition()
{
  for (auto & elem : _mesh.active_element_ptr_range())
    elem->set_extra_integer(_extra_integer_index, not_visited);
}

void
ClusteringUserObject::findCluster()
{

  std::stack<libMesh::Elem *> neighbor_stack;

  for (auto & elem : _mesh.active_element_ptr_range())
  {
    if (elem->get_extra_integer(_extra_integer_index) != NOT_VISITED)
      //if elem->get_extra_integer(_extra_integer_index) != NOT_VISITED that means we have
      //already touched that element. No need to visit it again.
      continue;

    int cluster_id = elem->id();
    neighbor_stack.push(elem);

    while (!neighbor_stack.empty())
    {
      libMesh::Elem * current_elem = neighbor_stack.top();
      neighbor_stack.pop();

      //Iterating through the side as, two elements to be clusted together they must share one side or face.
      for (unsigned int s = 0; s < current_elem->n_sides(); s++)
      {
        libMesh::Elem * neighbor_elem = current_elem->neighbor_ptr(s);
        //check if the neighbor_elem is
        //1. not a null ptr
        //2. an active element
        //3. NOT_VISITED before
        if (neighbor_elem && neighbor_elem->active() &&
            neighbor_elem->get_extra_integer(_extra_integer_index) == NOT_VISITED)
        {
          if (belongsToCluster(current_elem, neighbor_elem))
          {
            //if elements belong to a cluster then change the extra element integer
            //of both elements to the element id of the current element
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
ClusteringUserObject::getExtraIntegerScore(libMesh::Elem * elem) const
{
  return elem->get_extra_integer(_extra_integer_index);
}