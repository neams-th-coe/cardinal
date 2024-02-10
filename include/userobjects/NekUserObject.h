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
#include "GeometryUtils.h"

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
  virtual void execute() override;

  /**
   * Execute the user object; separating this call from execute() allows
   * all derived classes to leverage this base class's 'interval' parameter
   * to decide when to call the user object
   */
  virtual void executeUserObject() = 0;

protected:
  /// Interval with which to evaluate the user object
  const unsigned int & _interval;

  /// Underlying problem object
  const NekRSProblemBase * _nek_problem;

  /// Whether the mesh this userobject operates on is fixed, allowing caching of volumes and areas
  bool _fixed_mesh;
};
