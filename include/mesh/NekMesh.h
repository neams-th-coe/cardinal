//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef MESHGENERATORMESH_H
#define MESHGENERATORMESH_H

#include "MooseMesh.h"
#include "MooseApp.h"

/**
 * Mesh generated from parameters
 */
class NekMesh : public MooseMesh
{
public:
  NekMesh(const InputParameters & parameters);
  static InputParameters validParams();

  NekMesh(const NekMesh & /* other_mesh */) = default;

  // No copy
  NekMesh & operator=(const NekMesh & other_mesh) = delete;

  virtual std::unique_ptr<MooseMesh> safeClone() const override;

  virtual void buildMesh() override;
};

#endif /* MESHGENERATORMESH_H */
