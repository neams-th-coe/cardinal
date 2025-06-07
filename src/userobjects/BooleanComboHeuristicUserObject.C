#include "BooleanComboHeuristicUserObject.h"

registerMooseObject("CardinalApp", BooleanComboHeuristicUserObject);

InputParameters
BooleanComboHeuristicUserObject::validParams()
{

  static InputParameters params = ClusteringUserObject::validParams();
  params.addRequiredParam<std::vector<ExtraElementIDName>>(
      "extra_ids", " extra integer ids of the other heuristics");
  params.addRequiredParam<std::vector<std::string>>(
      "boolean_logic", " order of the boolean logic applied to the user objects");
  params.addClassDescription("Takes various heuristic user objects and"
                               " applies a user defined boolean logic operation on them.");
  return params;
}

BooleanComboHeuristicUserObject::BooleanComboHeuristicUserObject(const InputParameters & params)
  : ClusteringUserObject(params),
    _parsed_user_object_extra_element_id_names(
        getParam<std::vector<ExtraElementIDName>>("extra_ids")),
    _boolean_logic(getParam<std::vector<std::string>>("boolean_logic"))
{
  /* Need to ensure that the number of boolean logic is always
   * equals to == number of user object -1
   */
  for (auto id_name : _parsed_user_object_extra_element_id_names)
  {
    if (!_mesh.has_elem_integer(_id_name))
    {
      mooseError("Mesh does not have an extra element integer named ",
                 id_name,
                 "."
                 " Ensure your mesh generator defines it with extra_element_integers.");
    }
    _extra_integer_indexs.push_back(_mesh.get_elem_integer_index(id_name));
  }

  if (size(_parsed_user_object_extra_element_id_names) - 1 != size(_boolean_logic))
  {
      mooseError("The number of boolean logic operators is insconsistant:\n "
                 "expected = " + std::to_string(size(_parsed_user_object_extra_element_id_names) - 1) +"\n"+
                 "but  got = " + std::to_string(size(_boolean_logic)) + ".");
  }
  if (size(_parsed_user_object_extra_element_id_names) == 1 && size(_boolean_logic) == 0)
  {
    mooseWarning("No logic to apply ");
  }
}

bool
BooleanComboHeuristicUserObject::passesTheComboLogic(libMesh::Elem * element)
{

  std::vector<bool> logic_stack;
  for (auto extra_id : _extra_integer_indexs)
  {
    if (element->get_extra_integer(extra_id) != -1)
      logic_stack.push_back(true);
    else
      logic_stack.push_back(false);
  }
  
  std::vector<std::string> boolean_logic = _boolean_logic;
  bool decision;
  while (logic_stack.size() > 1)
  {

    if (boolean_logic[0] == "and")
      decision = logic_stack[0] and logic_stack[1];
    else
      decision = logic_stack[0] or logic_stack[1];
    logic_stack.erase(logic_stack.begin());
    boolean_logic.erase(boolean_logic.begin());
  }
  return decision;
}

bool
BooleanComboHeuristicUserObject::belongsToCluster(libMesh::Elem * base_element,
                                                  libMesh::Elem * neighbor_elem)
{
  return passesTheComboLogic(base_element) and passesTheComboLogic(neighbor_elem);
}