/********************************************************************/
/*                  SOFTWARE COPYRIGHT NOTIFICATION                 */
/*                             Cardinal                             */
/*                                                                  */
/*                  (c) 2021 UChicago Argonne, LLC                  */
/*                        ALL RIGHTS RESERVED                       */
/*                                                                  */
/*                 Prepared by UChicago Argonne, LLC                */
/*               Under Contract No. DE-AC02-06CH11357               */
/*                With the U. S. Department of Energy               */
/*                                                                  */
/*             Prepared by Battelle Energy Alliance, LLC            */
/*               Under Contract No. DE-AC07-05ID14517               */
/*                With the U. S. Department of Energy               */
/*                                                                  */
/*                 See LICENSE for full restrictions                */
/********************************************************************/

#pragma once

#include "GeneralPostprocessor.h"
#include "NekRSMesh.h"
#include "NekInterface.h"
#include "NekRSProblemBase.h"
#include "CardinalEnums.h"
#include "GeometryUtils.h"

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

protected:
  /// Base mesh this postprocessor acts on
  const MooseMesh & _mesh;

  /// Underlying NekRSMesh, if present
  const NekRSMesh * _nek_mesh;

  /// Underlying problem
  const NekRSProblemBase * _nek_problem;

  /// Which NekRS mesh to act on
  const nek_mesh::NekMeshEnum _pp_mesh;
};
