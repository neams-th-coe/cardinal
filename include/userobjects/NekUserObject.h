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

#include "ThreadedGeneralUserObject.h"
#include "NekRSProblemBase.h"
#include "CardinalEnums.h"

/**
 * Base class for providing common information to userobjects
 * operating directly on the NekRS solution and mesh.
 */
class NekUserObject : public ThreadedGeneralUserObject
{
public:
  static InputParameters validParams();

  NekUserObject(const InputParameters & parameters);

  virtual void initialize() {}
  virtual void finalize() {}

protected:
  /// Underlying problem object
  const NekRSProblemBase * _nek_problem;

  /// Base mesh this postprocessor acts on
  const MooseMesh & _mesh;

  /// Underlying NekRSMesh, if present
  const NekRSMesh * _nek_mesh;

  /// Whether the mesh this userobject operates on is fixed, allowing caching of volumes and areas
  bool _fixed_mesh;
};
