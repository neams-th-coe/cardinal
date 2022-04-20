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

#include "SecondOrderHexGenerator.h"
#include "CastUniquePointer.h"

#include "libmesh/cell_hex8.h"

registerMooseObject("CardinalApp", SecondOrderHexGenerator);

InputParameters
SecondOrderHexGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();

  params.addRequiredParam<MeshGeneratorName>("input", "The mesh we want to modify");

  params.addClassDescription(
      "Converts a HEX8 mesh to a HEX20 mesh, while optionally preserving "
      "circular edges (which were faceted) in the HEX8 mesh.");
  return params;
}

SecondOrderHexGenerator::SecondOrderHexGenerator(const InputParameters & params)
  : MeshGenerator(params), _input(getMesh("input"))
{
}

std::unique_ptr<MeshBase>
SecondOrderHexGenerator::generate()
{
  std::unique_ptr<MeshBase> mesh = std::move(_input);

  // loop over the mesh and store all the node information
  std::vector<Node> original_nodes;
  std::vector<dof_id_type> original_node_ids;
  for (const auto & node : mesh->node_ptr_range())
  {
    original_nodes.push_back(*node);
    original_node_ids.push_back(node->id());
  }

  // loop over the mesh and store all the element information
  int N = mesh->n_elem();
  std::vector<std::vector<libMesh::dof_id_type>> node_ids;
  std::vector<libMesh::dof_id_type> elem_ids;
  node_ids.resize(N);

  int i = 0;
  for (const auto & elem : mesh->element_ptr_range())
  {
    libMesh::Hex8 * hex8 = dynamic_cast<libMesh::Hex8 *>(elem);
    if (!hex8)
      mooseError("This mesh generator can only be applied to HEX8 elements!");

    elem_ids.push_back(hex8->set_id());

    for (unsigned int j = 0; j < 8; ++j)
      node_ids[i].push_back(hex8->node_ref(j).id());

    i++;
  }

  mesh->clear();

  // create the nodes
  for (unsigned int i = 0; i < original_nodes.size(); ++i)
  {
    Point pt(original_nodes[i](0), original_nodes[i](1), original_nodes[i](2));
    mesh->add_point(pt, original_node_ids[i]);
  }

  // create the elements
  for (unsigned int i = 0; i < elem_ids.size(); ++i)
  {
    auto elem = new Hex8;
    elem->set_id(elem_ids[i]);
    mesh->add_elem(elem);

    // node IDs for the i-th element
    const auto & ids = node_ids[i];

    std::cout << "element " << elem_ids[i] << " has nodes: " << ids.size() << std::endl;

    // create the original 8 nodes for hex20 (same numbering as hex8)
    for (unsigned int n = 0; n < ids.size(); ++n)
    {
      std::cout << ids[n] << std::endl;
      auto node_ptr = mesh->node_ptr(ids[n]);
      elem->set_node(n) = node_ptr;
    }
  }

  std::cout << "added all elemens" << std::endl;
  mesh->prepare_for_use();

  return dynamic_pointer_cast<MeshBase>(mesh);
}
