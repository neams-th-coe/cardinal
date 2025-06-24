#pragma once

#include "GeneralUserObject.h"

//forward declaration of the AuxiliarySystem
class AuxiliarySystem;

/*
 * Base class for clustering in cardinal.
 * */
class ClusteringUserObject : public GeneralUserObject
{

public:
  static InputParameters validParams();
  ClusteringUserObject(const InputParameters & parameters);

  virtual void execute() override;
  virtual void initialize() override {};
  virtual void finalize() override {};

  ///Getter function for the extra element integer value of an element
  int getExtraIntegerScore(libMesh::Elem * elem) const;

protected:

  ///Sets the extra element integer id of every active element in the mesh to NOT_VISITED
  void applyNoClusteringInitialCondition();

  ///Cluster finder method. It iterates thourgh the elements in the mesh.
  ///if two elements belongs to a cluster it changes the extra element integer of those two elements to a similar value.
  void findCluster();

  ///Get the metric data from the auxiliary system for an element.
  Real getMetricData(const libMesh::Elem * elem) const;

  ///A purely virtual fucntion which must be overrided in derived classes.
  ///It applies the clustering logic for two elements in the derived class
  virtual bool belongsToCluster(libMesh::Elem * base_element, libMesh::Elem * neighbor_elem) = 0;

  ///Holds the ExtraElementIDName
  const ExtraElementIDName _id_name;

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

  ///Extra element id index
  unsigned int _extra_integer_index = 0;

  ///If extra element integer value is equal to NOT_VISITED  that indicates
  ///an element has not yet been visited or assigned to a cluster while findCluster
  ///method is iterating through the elements in the mesh.
  static constexpr int NOT_VISITED = -1;
};