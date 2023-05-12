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

#include "NekPostprocessor.h"
#include "NekScalarValue.h"

/**
 * Displays the value held by a NekScalarValue userobject, for diagnosis
 * purposes.
 */
class NekScalarValuePostprocessor : public NekPostprocessor
{
public:
  static InputParameters validParams();

  NekScalarValuePostprocessor(const InputParameters & parameters);

  virtual Real getValue() override;

protected:
  /// Userobject to report contained value
  const NekScalarValue & _uo;
};
