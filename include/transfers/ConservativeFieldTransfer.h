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

#include "FieldTransferBase.h"

class ConservativeFieldTransfer : public FieldTransferBase
{
public:
  static InputParameters validParams();

  ConservativeFieldTransfer(const InputParameters & parameters);

protected:
  /**
   * Return a string to print in error messages upon failed normalizations
   * @return debugging hint
   */
  std::string normalizationHint() const;

  /// Absolute tolerance for checking conservation
  const Real & _abs_tol;

  /// Relative tolerance for checking conservation
  const Real & _rel_tol;

  /// Name of the postprocessor/vectorpostprocessor used to apply conservation
  std::string _postprocessor_name;
};
