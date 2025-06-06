#pragma once

#include "GeneralUserObject.h"

class AuxiliarySystem;

class ClusteringUserObject : public GeneralUserObject
{

public:
  static InputParameters validParams();
  ClusteringUserObject(const InputParameters & parameters);

  virtual void execute() override;
  virtual void initialize() override {};
  virtual void finalize() override {};
  int getExtraIntegerScore(libMesh::Elem * elem);

protected:
  void applyNoClusteringInitialCondition();
  void findCluster();
  Real getMetricData(const libMesh::Elem * elem);
  virtual bool belongsToCluster(libMesh::Elem * base_element, libMesh::Elem * neighbor_elem) = 0;

  const ExtraElementIDName _id_name;
  libMesh::MeshBase & _mesh;
  const AuxVariableName _metric_variable_name;
  MooseVariableBase & _metric_variable;
  AuxiliarySystem & _auxiliary_system;
  libMesh::DofMap & _dof_map;
  unsigned int _metric_variable_index;
  unsigned int _extra_integer_index = 0;
  static const int not_visited = -1;
};