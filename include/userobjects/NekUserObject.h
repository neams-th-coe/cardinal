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

  virtual void execute() {}
  virtual void initialize() {}
  virtual void finalize() {}

  virtual void threadJoin(const UserObject &) final {}
  virtual void subdomainSetup() final {}

  /**
   * Check whether a provided field is valid for this postprocessor
   * @param[in] field field
   */
  virtual void checkValidField(const field::NekFieldEnum & field) const;

protected:
  /// Underlying problem object
  const NekRSProblemBase * _nek_problem;

  /// Whether the mesh this userobject operates on is fixed, allowing caching of volumes and areas
  bool _fixed_mesh;
};
