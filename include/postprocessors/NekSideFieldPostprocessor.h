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

#include "NekSidePostprocessor.h"
#include "NekFieldInterface.h"

/**
 * Base class for all NekRS postprocessors that operate on fields
 * on boundaries of the NekRS domain, such as for averaging a field
 * variable on a sideset.
 */
class NekSideFieldPostprocessor : public NekSidePostprocessor, public NekFieldInterface
{
public:
  static InputParameters validParams();

  NekSideFieldPostprocessor(const InputParameters & parameters);
};
