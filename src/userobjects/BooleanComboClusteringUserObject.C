#include "BooleanComboClusteringUserObject.h"
#include "ClusteringHeuristicUserObjectBase.h"

registerMooseObject("CardinalApp", BooleanComboClusteringUserObject);

std::unordered_map<std::string, int> BooleanComboClusteringUserObject::_precedence{
    {"not", 3}, {"!", 3}, {"and", 2}, {"&&", 2}, {"or", 1}, {"||", 1}};

InputParameters
BooleanComboClusteringUserObject::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params.addRequiredParam<ExtraElementIDName>("id_name", "extra_element_integer_id name");
  params.addRequiredParam<std::vector<std::string>>("expression",
                                                    "boolean logic operation expression");
  params.addClassDescription("Takes various heuristic user objects and applies a user defined "
                             "boolean logic operation on them.");
  return params;
}

BooleanComboClusteringUserObject::BooleanComboClusteringUserObject(
    const InputParameters & parameters)
  : GeneralUserObject(parameters),
    _id_name(getParam<ExtraElementIDName>("id_name")),
    _mesh(_fe_problem.mesh().getMesh())
{
  if (!_mesh.has_elem_integer(_id_name))
  {
    mooseWarning(_id_name,
                 " extra element integer is missing in the mesh."
                 " Adding extra element integer ",
                 _id_name);
    _mesh.add_elem_integer(_id_name);
  }
  // check if mesh is replicated. If not then throw a moose error.
  if (!_mesh.is_replicated())
    mooseError("Mesh must be replicated");

  _extra_integer_index = _mesh.get_elem_integer_index(_id_name);
  reversePolishNotation(getParam<std::vector<std::string>>("expression"));
  initializeUserObjects();
}

void
BooleanComboClusteringUserObject::initializeUserObjects()
{
  _clustering_user_objects.clear();
  for (const auto & token : _output_stack)
  {
    if (_precedence.count(token))
      // separate the user object names. If true that means name is an operator
      continue;
    const auto & uo = getUserObjectByName<ClusteringHeuristicUserObjectBase>(token);
    _clustering_user_objects.insert(std::make_pair(token, &uo));
  }
}

bool
BooleanComboClusteringUserObject::belongsToCluster(libMesh::Elem * base_element,
                                                   libMesh::Elem * neighbor_elem)
{
  // follow the reverse polish notation
  std::stack<bool> result_stack;

  for (const auto token : _output_stack)
  {
    // if token is an operator
    if (token == "and" || token == "&&")
    {
      bool rhs = result_stack.top();
      result_stack.pop();
      bool lhs = result_stack.top();
      result_stack.pop();
      result_stack.push(lhs && rhs);
    }
    else if (token == "or" || token == "||")
    {
      bool rhs = result_stack.top();
      result_stack.pop();
      bool lhs = result_stack.top();
      result_stack.pop();
      result_stack.push(lhs || rhs);
    }
    else if (token == "not" || token == "!")
    {
      bool val = result_stack.top();
      result_stack.pop();
      result_stack.push(!val);
    }
    else
      result_stack.push(_clustering_user_objects[token]->evaluate(base_element, neighbor_elem));
  }
  return result_stack.top();
}

void
BooleanComboClusteringUserObject::findCluster()
{
  std::stack<libMesh::Elem *> neighbor_stack;

  for (auto & elem : _mesh.active_element_ptr_range())
  {
    if (elem->get_extra_integer(_extra_integer_index) != NOT_VISITED)
      continue;

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
            neighbor_elem->get_extra_integer(_extra_integer_index) == NOT_VISITED)
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

void
BooleanComboClusteringUserObject::resetExtraInteger()
{
  for (auto & elem : _mesh.active_element_ptr_range())
    elem->set_extra_integer(_extra_integer_index, NOT_VISITED);
}

void
BooleanComboClusteringUserObject::execute()
{
  resetExtraInteger();
  findCluster();
}

int
BooleanComboClusteringUserObject::getExtraIntegerScore(libMesh::Elem * elem) const
{
  return elem->get_extra_integer(_extra_integer_index);
}

void
BooleanComboClusteringUserObject::reversePolishNotation(const std::vector<std::string> & expression)
{
  std::stack<std::string> op_stack;

  for (const auto & token : expression)
  {
    if (token == _left_parenthesis)
      op_stack.push(token);
    else if (token == _right_parenthesis)
    {
      while (!op_stack.empty() && op_stack.top() != _left_parenthesis)
      {
        _output_stack.push_back(op_stack.top());
        op_stack.pop();
      }
      if (!op_stack.empty() && op_stack.top() == _left_parenthesis)
        op_stack.pop();
    }
    // operator handling based on _precedence
    else if (_precedence.find(token) != _precedence.end())
    {
      // if operation hasn't the least precedence
      // push back to the output stack
      while (!op_stack.empty() && _precedence.find(op_stack.top()) != _precedence.end() &&
             _precedence[op_stack.top()] >= _precedence[token])
      {
        _output_stack.push_back(op_stack.top());
        op_stack.pop();
      }
      op_stack.push(token);
    }
    else
      _output_stack.push_back(token);
  }
  while (!op_stack.empty())
  {
    _output_stack.push_back(op_stack.top());
    op_stack.pop();
  }
}
