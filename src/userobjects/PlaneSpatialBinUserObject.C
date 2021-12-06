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

#include "PlaneSpatialBinUserObject.h"

InputParameters
PlaneSpatialBinUserObject::validParams()
{
  InputParameters params = SpatialBinUserObject::validParams();
  return params;
}

PlaneSpatialBinUserObject::PlaneSpatialBinUserObject(const InputParameters & parameters)
  : SpatialBinUserObject(parameters)
{
}
