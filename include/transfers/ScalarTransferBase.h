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

#include "NekTransferBase.h"

/// Base class for transferring scalars (single numbers) between NekRS and MOOSE
class ScalarTransferBase : public NekTransferBase
{
public:
  static InputParameters validParams();

  ScalarTransferBase(const InputParameters & parameters);

  /**
   * Get the usrwrk slot that this transfer is using
   * @return usrwrk slot
   */
  const unsigned int usrwrkSlot() const { return _usrwrk_slot; }

  /**
   * Get the offset in the slot where the single value is written
   * @return offset
   */
  const unsigned int offset() const { return _offset; }

  /**
   * Get the multiplicative scaling applied
   * @return scaling
   */
  const Real & scaling() const { return _scaling; }

protected:
  /// A multiplier to apply to the value passed in
  const Real & _scaling;

  /// Slot in usrwrk to write the scalar value
  unsigned int _usrwrk_slot;

  /// Offset in the slot to write the scalar value
  unsigned int _offset;

  /**
   * To allow multiple scalar numbers to be written into the same usrwrk slot
   * (for memory efficiency), we allow multiple ScalarTransfers to write into
   * the same slot. They are written by automatically incrementing each one by
   * 1; this static variable keeps track of the last used offset in a given slot.
   * Ordered as (usrwrk slot, last occupied entry)
   */
  static std::map<unsigned int, unsigned int> _counter;
};
