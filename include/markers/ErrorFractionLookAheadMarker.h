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

#include "ErrorFractionMarker.h"

#include "OpenMCBase.h"

class ErrorFractionLookAheadMarker : public ErrorFractionMarker,
                                     public OpenMCBase
{
public:
  static InputParameters validParams();

  ErrorFractionLookAheadMarker(const InputParameters & parameters);

protected:
  virtual MarkerValue computeElementMarker() override;

  /**
   * The variables containing the tally relative error. This needs to be a vector because the tally
   * may have filters applied, and so we need to take the max over all filter bins.
   */
  std::vector<const VariableValue *> _tally_rel_error;

  /// Upper relative error limit for refinement. If the "lookahead" for an element exceeds this
  /// limit, don't refine. Otherwise, mark for refinement.
  const Real & _rel_error_limit;
};
