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


#include "ParsedElementIDMeshGenerator.h"

registerMooseObject("CardinalApp", ParsedElementIDMeshGenerator);

InputParameters
ParsedElementIDMeshGenerator::validParams()
{

  InputParameters params = MeshGenerator::validParams();
  params.addRequiredParam<MeshGeneratorName>("input", "The mesh we want to modify");
  params.addRequiredParam<std::vector<ExtraElementIDName>>("extra_element_integer_names",
                                                           "list of extra integer names to"
                                                           "to be added in the mesh.");
  params.addParam<std::vector<int>>("values",
                                    "default values of the "
                                    "extra_element_integer_names");
  params.addClassDescription("A MeshGenerator Object which just adds "
                             "extra element integers to the whole mesh.");
  return params;
}

ParsedElementIDMeshGenerator::ParsedElementIDMeshGenerator(const InputParameters & params)
  : MeshGenerator(params),
    _input(getMesh("input")),
    _extra_element_id_names(getParam<std::vector<ExtraElementIDName>>("extra_element_integer_names")),
    _eeiid_values(isParamValid("values") ?
                                         getParam<std::vector<int>>("values") :
                                         std::vector<int>(_extra_element_id_names.size(), -1))
{
  //check if value for every extra element integer is provided
  if (_eeiid_values.size() != _extra_element_id_names.size())
    mooseError("Number of values must match number of extra element integer names");
}

std::unique_ptr<MeshBase>
ParsedElementIDMeshGenerator::generate()
{
  std::unique_ptr<MeshBase> mesh = std::move(_input);
  if (!mesh)
    mooseError("Input mesh is null");

  for (int i = 0; i < _extra_element_id_names.size(); i++)
  {
    //check if the extra element integer alreadys exists in the mesh and only add the
    //if it doesn't. If it exits already then throw a mooseWarning.
    if (!mesh->has_elem_integer(_extra_element_id_names[i]))
      mesh->add_elem_integer(_extra_element_id_names[i], _eeiid_values[i]);
    else
      mooseWarning("The element integer id named ", _extra_element_id_names[i], " already exists in the mesh. The existing values will be retained, and the provided values will be ignored.");

  }

  return mesh;
}
