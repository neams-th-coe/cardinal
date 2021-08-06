//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralPostprocessor.h"
#include "NekRSMesh.h"
#include "NekInterface.h"
#include "NekRSProblemBase.h"
#include "CardinalEnums.h"

class NekPostprocessor;

template <>
InputParameters validParams<NekPostprocessor>();

/**
 * Base class for providing common information to postprocessors
 * operating directly on the nekRS solution and mesh.
 */
class NekPostprocessor : public GeneralPostprocessor
{
public:
  static InputParameters validParams();

  NekPostprocessor(const InputParameters & parameters);

  virtual void initialize() override {}
  virtual void execute() override {}

  /**
   * Check whether a provided field is valid for this postprocessor
   * @param[in] field field
   */
  virtual void checkValidField(const field::NekFieldEnum & field) const;

protected:
  /// Base mesh this postprocessor acts on
  const MooseMesh & _mesh;

  /// Whether the mesh this postprocessor operates on is fixed, allowing caching of volumes and areas
  bool _fixed_mesh;

  /// Underlying NekRSMesh, if present
  const NekRSMesh * _nek_mesh;

  /// Underlying problem
  const NekRSProblemBase * _nek_problem;
};
