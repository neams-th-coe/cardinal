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

#include "OpenMCProblemBase.h"

InputParameters
OpenMCProblemBase::validParams()
{
  InputParameters params = ExternalProblem::validParams();
  params.addRequiredRangeCheckedParam<Real>("power", "power >= 0.0",
    "Power (Watts) to normalize the OpenMC tallies");
  params.addParam<bool>("verbose", false, "Whether to print diagnostic information");
  return params;
}

OpenMCProblemBase::OpenMCProblemBase(const InputParameters &params) :
  ExternalProblem(params),
  _power(getParam<Real>("power")),
  _verbose(getParam<bool>("verbose"))
{
}

OpenMCProblemBase::~OpenMCProblemBase()
{
}
