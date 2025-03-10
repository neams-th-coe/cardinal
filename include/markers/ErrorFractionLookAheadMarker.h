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

#include "ErrorFractionMarker.h"

#include "OpenMCBase.h"

class ErrorFractionLookAheadMarker : public ErrorFractionMarker,
                                     public OpenMCBase
{
public:
  static InputParameters validParams();

  ErrorFractionLookAheadMarker(const InputParameters & parameters);

  virtual void markerSetup() override;

protected:
  virtual MarkerValue computeElementMarker() override;

  /// Upper relative error limit for refinement. If the "lookahead" for an element exceeds this
  /// limit, don't refine. Otherwise, mark for refinement.
  const Real & _rel_error_limit;

  /// The relative error reported by an indicator.
  ErrorVector & _rel_error_vec;

  /// The exponent applied to the number of children of the element.
  enum class LookAheadOrder
  {
    Constant = 0,
    Root = 1,
    Linear = 2,
    Quadratic = 3
  } _order;
};
