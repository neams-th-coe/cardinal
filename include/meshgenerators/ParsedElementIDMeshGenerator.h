#pragma once

#include "MeshGenerator.h"

/*
 * MeshGenerator that assigns single integer values to user-defined extra element IDs
 * across the entire mesh.
 */

class ParsedElementIDMeshGenerator : public MeshGenerator
{

public:
  static InputParameters validParams();

  ParsedElementIDMeshGenerator(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

protected:
  /// MeshBase object where extra element integer ids will be added
  std::unique_ptr<MeshBase> & _input;

  /// Names of the extra element integer ids used
  const std::vector<ExtraElementIDName> & _extra_element_id_names;

  /// values of the extra element integer ids used
  std::vector<int> _eeiid_values;
};
