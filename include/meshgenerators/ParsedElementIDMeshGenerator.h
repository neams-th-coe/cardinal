#pragma once

#include "MeshGenerator.h"

/*
 * MeshGenerator that assigns constant integer values to user-defined extra element IDs
 * across the entire mesh.
 */

class ParsedElementIDMeshGenerator : public MeshGenerator
{

public:
  static InputParameters validParams();

  ParsedElementIDMeshGenerator(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

protected:
  /// MeshBase object where eeid will be added
  std::unique_ptr<MeshBase> & _input;

  /// Names of the extra element ids used
  const std::vector<ExtraElementIDName> & _extra_element_id_names;

  /// values of the extra element ids used
  std::vector<int> _eeiid_values;
};
