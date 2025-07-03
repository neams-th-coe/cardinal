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

#include "CardinalEnums.h"

/// Transfers field data (NekRS solution fields) between NekRS and MOOSE
class NekFieldVariable : public FieldTransferBase
{
public:
  static InputParameters validParams();

  NekFieldVariable(const InputParameters & parameters);

  ~NekFieldVariable();

  virtual void readDataFromNek() override;

  virtual void sendDataToNek() override;

protected:
  /**
   * Convert from the field string into the enumeration used internally
   * in the NekInterface system.
   * @param[in] name field name
   * @return field enumeration
   */
  field::NekFieldEnum convertToFieldEnum(const std::string name) const;

  /// NekRS field to read/write
  field::NekFieldEnum _field;

  /// Scratch space to place external NekRS fields before writing into auxiliary variables
  double * _external_data = nullptr;
};
