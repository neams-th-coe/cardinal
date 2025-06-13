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

#include "ScalarTransferBase.h"

/// Pass scalar values (single numbers) between NekRS and MOOSE
class NekScalarValue : public ScalarTransferBase
{
public:
  static InputParameters validParams();

  NekScalarValue(const InputParameters & parameters);

  virtual void sendDataToNek() override;

protected:
  /// The value to pass into NekRS, possibly multiplied by 'scaling'
  const Real & _value;

  /// Name of postprocessor to output the value sent into NekRS, for diagnostics
  const PostprocessorName * _postprocessor;
};
