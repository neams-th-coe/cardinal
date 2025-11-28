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

#include "GeneralPostprocessor.h"

#include "OpenMCBase.h"
#include "CardinalEnums.h"
#include "TallyBase.h"

/**
 * Compute the max/min relative error of the tally coupling OpenMC to MOOSE.
 */
class TallyRelativeError : public GeneralPostprocessor, public OpenMCBase
{
public:
  static InputParameters validParams();

  TallyRelativeError(const InputParameters & parameters);

  virtual void initialize() override {}
  virtual void execute() override {}

  virtual Real getValue() const override;

protected:
  /// type of extrema operation
  const operation::OperationEnum _type;

  /// The tally score we want to fetch the relative error of.
  std::string _score;

  /// The tally we're computing the relative error from.
  const TallyBase * _tally;
};
