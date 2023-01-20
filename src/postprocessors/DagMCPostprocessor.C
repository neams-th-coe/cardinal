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

#ifdef ENABLE_DAGMC

#include "DagMCPostprocessor.h"

registerMooseObject("CardinalApp", DagMCPostprocessor);

InputParameters
DagMCPostprocessor::validParams()
{
  InputParameters params = OpenMCPostprocessor::validParams();
  return params;
}

DagMCPostprocessor::DagMCPostprocessor(const InputParameters & parameters)
  : OpenMCPostprocessor(parameters)
{
  mooseError("This class solely exists to have an object for checking whether "
    "to run DagMC related tests");
}

Real
DagMCPostprocessor::getValue()
{
  return 0.0;
}

#endif
