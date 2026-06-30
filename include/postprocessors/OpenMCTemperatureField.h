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

#include "GeneralVectorPostprocessor.h"

class OpenMCTemperatureField : public GeneralVectorPostprocessor
{
public:
  static InputParameters validParams();
  OpenMCTemperatureField(const InputParameters & params);

  virtual void initialize() override;
  virtual void execute() override;
  virtual void finalize() override {}

private:
  VectorPostprocessorValue & _cell_id;
  VectorPostprocessorValue & _temperature;
};
