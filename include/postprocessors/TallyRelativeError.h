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

#include "OpenMCPostprocessor.h"
#include "CardinalEnums.h"

/**
 * Compute the max/min relative error of the tally coupling OpenMC to MOOSE.
 */
class TallyRelativeError : public OpenMCPostprocessor
{
public:
  static InputParameters validParams();

  TallyRelativeError(const InputParameters & parameters);

  virtual Real getValue() const override;

protected:
  /// type of extrema operation
  const operation::OperationEnum _type;

  /// index of score in _tally_score of the problem
  int _tally_index;
};
