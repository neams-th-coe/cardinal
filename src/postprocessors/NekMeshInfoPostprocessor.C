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

#ifdef ENABLE_NEK_COUPLING

#include "NekMeshInfoPostprocessor.h"
#include "SubProblem.h"
#include "MooseMesh.h"

registerMooseObject("CardinalApp", NekMeshInfoPostprocessor);

InputParameters
NekMeshInfoPostprocessor::validParams()
{
  InputParameters params = NekPostprocessor::validParams();

  MooseEnum test_type("num_elems num_nodes node_x node_y node_z");
  params.addRequiredParam<MooseEnum>("test_type",
                                     test_type,
                                     "The type of info to fetch; "
                                     "this is used to toggle between many different tests to avoid "
                                     "creating tons of source files.");

  params.addParam<libMesh::dof_id_type>("node", "Element-local node ID");
  params.addParam<Point>("point", "Point used to locate element");

  params.addClassDescription("Perform various tests on the construction of a nekRS mesh on "
                             "a particular boundary to give a surface mesh.");
  return params;
}

NekMeshInfoPostprocessor::NekMeshInfoPostprocessor(const InputParameters & parameters)
  : NekPostprocessor(parameters), _test_type(getParam<MooseEnum>("test_type"))
{
  if (!_nek_mesh)
    mooseError("This class is intended for testing the 'NekRSMesh' mesh, "
               "and cannot be used with other mesh types.");

  // For any of the node_x, node_y, and node_z settings, we need to grab an element ID
  // and an element-local node ID
  if (_test_type == "node_x" || _test_type == "node_y" || _test_type == "node_z")
  {
    auto locator = _nek_mesh->getPointLocator();

    if (!isParamValid("point"))
      paramError("point", "When using a node test, a point must be specified to locate an element");

    if (!isParamValid("node"))
      paramError("node",
                 "A 'node' must be specified when the 'test_type' is "
                 "'node_x', 'node_y', or 'node_z'.");

    const Point & p = getParam<Point>("point");
    _element = (*locator)(p);

    bool found_element = _element;
    getMooseApp().getCommunicator()->max(found_element);

    if (!found_element)
      paramError("point", "The specified point cannot be found in the mesh");

    _node = &getParam<libMesh::dof_id_type>("node");

    if (*_node >= _nek_mesh->nNodes() / _nek_mesh->nElem())
      paramError("node", "The 'node' must be in the range [0, number of nodes / element]!");
  }
}

Real
NekMeshInfoPostprocessor::getValue() const
{

  if (_test_type == "num_elems")
    return _nek_mesh->nElem();
  else if (_test_type == "num_nodes")
    return _nek_mesh->nNodes();
  else if (_test_type == "node_x" || _test_type == "node_y" || _test_type == "node_z")
  {
    int id = _test_type == "node_x" ? 0 : (_test_type == "node_y" ? 1 : 2);

    Real coord;
    libMesh::processor_id_type p_id = 0;
    const auto comm = getMooseApp().getCommunicator();

    if (_element)
    {
      auto node = _element->node_ptr(*_node);
      coord = (*node)(id);
      p_id = _element->processor_id();
    }

    // can get the processor ID of the owning rank by just finding maximum, since
    // guaranteed rank >= 0
    comm->max(p_id);

    comm->broadcast(coord, p_id);

    return coord;
  }
  else
    mooseError("Unhandled 'test_type' enum!");
}

#endif
