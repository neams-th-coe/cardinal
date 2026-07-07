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

#include "MooseObject.h"
#include "OpenMCBase.h"
#include "CardinalEnums.h"

/**
 * Applies a modification to the OpenMC model prior to execution.
 */
class ModelModifiersBase : public MooseObject, public OpenMCBase
{
public:
  static InputParameters validParams();

  ModelModifiersBase(const InputParameters & parameters);

  /// Modify the OpenMC model
  virtual void modifyOpenMCModel() const = 0;
};
