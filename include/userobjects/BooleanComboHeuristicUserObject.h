#pragma once

#include "ClusteringUserObject.h"

class BooleanComboHeuristicUserObject : public ClusteringUserObject
{

public:
  static InputParameters validParams();
  BooleanComboHeuristicUserObject(const InputParameters & parameters);

protected:
  virtual bool belongsToCluster(libMesh::Elem * base_elem, libMesh::Elem * current_elem) override;

private:
  bool passesTheComboLogic(libMesh::Elem * element);
  std::vector<ExtraElementIDName> _parsed_user_object_extra_element_id_names;
  std::vector<std::string> _boolean_logic;
  std::vector<unsigned int> _extra_integer_indexs;
};