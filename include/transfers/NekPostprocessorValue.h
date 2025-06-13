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

/// Passes postprocessor values between NekRS and MOOSE
class NekPostprocessorValue : public ScalarTransferBase
{
public:
  static InputParameters validParams();

  NekPostprocessorValue(const InputParameters & parameters);

  virtual void sendDataToNek() override;

protected:
  /// Postprocessor used to send values into NekRS
  const PostprocessorValue * _postprocessor = nullptr;
};
