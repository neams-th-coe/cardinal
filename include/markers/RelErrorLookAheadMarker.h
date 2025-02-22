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

#include "QuadraturePointMarker.h"

class RelErrorLookAheadMarker : public QuadraturePointMarker
{
public:
  static InputParameters validParams();

  RelErrorLookAheadMarker(const InputParameters & parameters);

protected:
  virtual MarkerValue computeQpMarker() override;

  /// Upper relative error limit for refinement. If the "lookahead" for an element exceeds this
  /// limit, don't refine. Otherwise, mark for refinement.
  const Real & _rel_error_limit;
};
