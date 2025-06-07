#pragma once

#include "MeshGenerator.h"

class ParsedElementIDMeshGenerator : public MeshGenerator
{

public:
  static InputParameters validParams();
  ParsedElementIDMeshGenerator(const InputParameters & parameters);
  std::unique_ptr<MeshBase> generate() override;

protected:
  std::unique_ptr<MeshBase>& _input;
  std::vector<ExtraElementIDName> _extra_element_id_names;
  std::vector<int> _eeiid_values;
};