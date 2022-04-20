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

  // loop over all elements
  for (const auto & elem : mesh->element_ptr_range())
  {
    // each element must be HEX8
    const libMesh::Hex8 * elem8 = dynamic_cast<const libMesh::Hex8 *>(elem);
    if (!elem8)
      mooseError("This mesh generator can only be applied to HEX8 elements!");
  }

  return dynamic_pointer_cast<MeshBase>(mesh);
}
