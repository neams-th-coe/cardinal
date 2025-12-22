#pragma once

#include "GeneralUserObject.h"

class AuxiliarySystem;

/* Base class for clustering in cardinal. */
class ClusteringHeuristicUserObjectBase : public GeneralUserObject
{

public:
  static InputParameters validParams();
  ClusteringHeuristicUserObjectBase(const InputParameters & parameters);

  virtual void execute() override {};
  virtual void initialize() override;
  virtual void finalize() override {};

  /**
   * A purely virtual function which must be overridden in derived classes.
   * It applies the clustering logic for two elements in the derived class
   * param[in] base_element the current element
   * param[in] neighbor_elem the current neighbour of base_element
   * return whether the two elements should be added to a cluster or not
   */
  virtual bool evaluate(libMesh::Elem * base_element, libMesh::Elem * neighbor_elem) const = 0;

protected:
  /**
   * Get the metric data from the auxiliary system for an element.
   * @param[in] an element
   * @param[out] real value of the _metric_variable
   */
  Real getMetricData(const libMesh::Elem * elem) const;

  /**
   * Gathers metric data from all processors during initialization
   */
  void gatherAllMetricData();

  /// Mesh reference
  libMesh::MeshBase & _mesh;

  /// Name of the metric variable based on which clustering is done
  const AuxVariableName _metric_variable_name;

  /// Metric variable
  MooseVariableBase & _metric_variable;

  /// AuxiliarySystem reference
  AuxiliarySystem & _auxiliary_system;

  /// DOF map
  libMesh::DofMap & _dof_map;

  /// Metric variable index
  unsigned int _metric_variable_index;

  /// Whether the global metric data has been gathered
  bool _data_gathered;

  /// libmesh numeric vector with serialized solutions
  NumericVector<Real> & _serialized_metric_solution;
};
