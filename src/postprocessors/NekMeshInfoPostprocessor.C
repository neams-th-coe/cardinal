#include "NekMeshInfoPostprocessor.h"
#include "SubProblem.h"
#include "MooseMesh.h"
#include "NekMesh.h"

registerMooseObject("NekApp", NekMeshInfoPostprocessor);

template <>
InputParameters
validParams<NekMeshInfoPostprocessor>()
{
  InputParameters params = validParams<GeneralPostprocessor>();

  MooseEnum test_type("num_elems num_nodes node_x node_y node_z");
  params.addRequiredParam<MooseEnum>("test_type", test_type, "The type of info to fetch; "
    "this is used to toggle between many different tests to avoid creating tons of source files.");

  params.addParam<int>("element", "Element ID in NekMesh");
  params.addParam<int>("node", "Element-local node ID");

  params.addClassDescription("Perform various tests on the construction of a nekRS mesh on "
    "a particular boundary to give a surface mesh.");
  return params;
}

NekMeshInfoPostprocessor::NekMeshInfoPostprocessor(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
   _test_type(getParam<MooseEnum>("test_type")),
   _mesh(_subproblem.mesh())
{
  _nek_mesh = dynamic_cast<const NekRSMesh *>(&_mesh);

  if (!_nek_mesh)
    mooseError("'NekMeshInfoPostprocessor' is intended for testing the 'NekRSMesh' mesh, "
      "and cannot be used with other mesh types.");

  // For any of the node_x, node_y, and node_z settings, we need to grab an element ID
  // and an element-local node ID
  if (_test_type == "node_x" || _test_type == "node_y" || _test_type == "node_z")
  {
    _element = &getParam<int>("element");
    _node = &getParam<int>("node");

    if (!_element)
      paramError("element", "An 'element' must be specified when the 'test_type' is "
        "'node_x', 'node_y', or 'node_z'.");

    if (!_node)
      paramError("node", "A 'node' must be specified when the 'test_type' is "
        "'node_x', 'node_y', or 'node_z'.");

    if (*_element < 0 || *_element >= _nek_mesh->nElem())
      paramError("element", "The 'element' must be in the range [0, number of elements]!");

    if (*_node < 0 || *_node >= _nek_mesh->nNodes() / _nek_mesh->nElem())
      paramError("node", "The 'node' must be in the range [0, number of nodes / element]!");
  }

}

Real
NekMeshInfoPostprocessor::getValue()
{
  if (_test_type == "num_elems")
    return _nek_mesh->nElem();
  else if (_test_type == "num_nodes")
    return _nek_mesh->nNodes();
  else if (_test_type == "node_x")
  {
    auto element = _nek_mesh->elemPtr(*_element);
    auto node = element->node_ptr(*_node);
    return (*node)(0);
  }
  else if (_test_type == "node_y")
  {
    auto element = _nek_mesh->elemPtr(*_element);
    auto node = element->node_ptr(*_node);
    return (*node)(1);
  }
  else if (_test_type == "node_z")
  {
    auto element = _nek_mesh->elemPtr(*_element);
    auto node = element->node_ptr(*_node);
    return (*node)(2);
  }
  else
    mooseError("Unhandled 'test_type' enum in 'NekMeshInfoPostprocessor'!");
}
