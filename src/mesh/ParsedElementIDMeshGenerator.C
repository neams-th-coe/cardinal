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
    if (!mesh->has_elem_integer(_extra_element_id_names[i]))
      mesh->add_elem_integer(_extra_element_id_names[i], _eeiid_values[i]);
  }

  return mesh;
}
