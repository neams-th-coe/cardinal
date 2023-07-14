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

#include "GeneralUserObject.h"

/**
 * Opaque user object which will send a scalar value into NekRS through its
 * scratch space. The user then has infinite flexibility to use that value
 * in the NekRS case files.
 */
class NekScalarValue : public GeneralUserObject
{
public:
  static InputParameters validParams();

  NekScalarValue(const InputParameters & parameters);

  /// We don't want this user object to execute in MOOSE's control
  virtual void execute() override {}

  virtual void initialize() override {}
  virtual void finalize() override {}

  /// Instead, we want to have a separate method that we can call from NekRSProblemBase
  virtual void setValue();

  /**
   * Get the value currently held by this user object
   * @return value
   */
  Real getValue() const;

  virtual unsigned int usrwrkSlot() const { return _usrwrk_slot; }

  virtual void setCounter(const unsigned int & counter) { _counter = counter; }

  /**
   * Get the index in usrwrk for which this scalar value is held
   * @return usrwrk index
   */
  virtual const unsigned int & counter() const { return _counter; }

protected:
  /// The value to send into NekRS
  const Real & _value;

  /// Slot in usrwrk to write the scalar value
  unsigned int _usrwrk_slot;

  /// Counter of this object, to be set by NekRSProblem
  unsigned int _counter;
};
