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

/**
 * This postprocessor is used to extract running information of NekRS results,
 * like number of iteration in velocity solver, CFL, etc.
 */
class NekInfoPostprocessor : public NekPostprocessor
{
public:
  NekInfoPostprocessor(const InputParameters & parameters);

  static InputParameters validParams();

  virtual void initialize() override {}
  virtual void execute() override {}

  virtual Real getValue() const override;

private:
  /// Type of value to compute
  const MooseEnum _test_type;
};

