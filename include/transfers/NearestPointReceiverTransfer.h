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

#include "MultiAppTransfer.h"

// Forward declarations
class NearestPointReceiverTransfer;

template <>
InputParameters validParams<NearestPointReceiverTransfer>();

/**
 * Copies the value of a Postprocessor from the Master to a MultiApp.
 */
class NearestPointReceiverTransfer : public MultiAppTransfer
{
public:
  NearestPointReceiverTransfer(const InputParameters & parameters);

  virtual void execute() override;

protected:
  UserObjectName _from_uo_name;
  UserObjectName _to_uo_name;
};
