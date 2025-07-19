#pragma once

#include "GeneralUserObject.h"

class AuxiliarySystem;

/* Base class for clustering in cardinal. */
class ClusteringUserObject : public GeneralUserObject
{

public:
  static InputParameters validParams();
  ClusteringUserObject(const InputParameters & parameters);

  virtual void execute() override {};
  virtual void initialize() override {};
  virtual void finalize() override {};

protected:
  ///Get the metric data from the auxiliary system for an element.
  Real getMetricData(const libMesh::Elem * elem) const;

  /**
   * A purely virtual function which must be overrided in derived classes.
   * It applies the clustering logic for two elements in the derived class
   */
  virtual bool belongsToCluster(libMesh::Elem * base_element, libMesh::Elem * neighbor_elem) const = 0;

  ///Mesh reference
  libMesh::MeshBase & _mesh;

  ///Name of the metric variable based on which clustering is done
  const AuxVariableName _metric_variable_name;

  ///Metric variable
  MooseVariableBase & _metric_variable;

  ///AuxiliarySystem reference
  AuxiliarySystem & _auxiliary_system;

  ///DOF map
  libMesh::DofMap & _dof_map;

  ///Metric variable index
  unsigned int _metric_variable_index;
};