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

  params.addParam<std::vector<int>>(
      "values",
      "Optional list of integer values corresponding to each name in "
      "'extra_element_integer_names'. "
      "If not provided, all values will default to -1. "
      "If provided, the list must contain the same number of entries as "
      "'extra_element_integer_names', "
      "with each value assigned to the respective extra element integer.");

  params.addClassDescription("A MeshGenerator Object which just adds "
                             "extra element integers to the whole mesh.");
  return params;
}

ParsedElementIDMeshGenerator::ParsedElementIDMeshGenerator(const InputParameters & params)
  : MeshGenerator(params),
    _input(getMesh("input")),
    _extra_element_id_names(
        getParam<std::vector<ExtraElementIDName>>("extra_element_integer_names")),
    _eeiid_values(isParamValid("values") ? getParam<std::vector<int>>("values")
                                         : std::vector<int>(_extra_element_id_names.size(), -1))
{
  // check if value for every extra element integer is provided
  if (_eeiid_values.size() != _extra_element_id_names.size())
    paramError("values",
               "Number of entries in 'values' (" + std::to_string(_eeiid_values.size()) +
                   ") must match the number of entries in 'extra_element_integer_names'"
                   " (" +
                   std::to_string(_extra_element_id_names.size()) + ")");
}

std::unique_ptr<MeshBase>
ParsedElementIDMeshGenerator::generate()
{
  std::unique_ptr<MeshBase> mesh = std::move(_input);

  for (int i = 0; i < _extra_element_id_names.size(); i++)
  {
    // check if the extra element integer already exists in the mesh and only add the
    // element integer if it doesn't.
    // If it exits already then throw a mooseError.
    if (!mesh->has_elem_integer(_extra_element_id_names[i]))
      mesh->add_elem_integer(_extra_element_id_names[i], _eeiid_values[i]);
    else
      mooseError("The element integer id named ",
                 _extra_element_id_names[i],
                 " already exists in the mesh.");
  }

  return mesh;
}
