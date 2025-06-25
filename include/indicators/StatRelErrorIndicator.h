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

#include "OpenMCIndicator.h"

/// An Indicator which returns the statistical error of a tally score in an element.
class StatRelErrorIndicator : public OpenMCIndicator
{
public:
  static InputParameters validParams();

  StatRelErrorIndicator(const InputParameters & parameters);

  virtual void computeIndicator() override;

protected:
  /// The external filter bin index for the score.
  const unsigned int _bin_index;

  /// The variables containing the tally score relative errors.
  const VariableValue * _tally_rel_error;
};
