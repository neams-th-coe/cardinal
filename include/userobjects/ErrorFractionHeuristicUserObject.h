#pragma once

#include "ClusteringUserObject.h"

class ErrorFractionHeuristicUserObject : public ClusteringUserObject
{

public:
  static InputParameters validParams();
  ErrorFractionHeuristicUserObject(const InputParameters & params);

protected:
  virtual bool belongsToCluster(libMesh::Elem * base_element,
                                libMesh::Elem * neighbor_elem) override;
  virtual void execute() override;
  void extremesFinder();

  Real _upper_fraction;
  Real _lower_fraction;
    Real _max;
    Real _min;

  Real _upper_cut_off;
  Real _lower_cut_off;
};