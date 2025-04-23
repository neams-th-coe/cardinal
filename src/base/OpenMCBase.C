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

#ifdef ENABLE_OPENMC_COUPLING

#include "OpenMCBase.h"

#include "libmesh/elem.h"

InputParameters
OpenMCBase::validParams()
{
  InputParameters params = emptyInputParameters();
  return params;
}

OpenMCBase::OpenMCBase(const MooseObject * moose_object, const InputParameters & parameters)
  : _openmc_problem(
        dynamic_cast<OpenMCCellAverageProblem *>(&moose_object->getMooseApp().feProblem()))
{
  if (!_openmc_problem)
    mooseError(moose_object->type() +
               " can only be used with problems of type 'OpenMCCellAverageProblem'!");
}

Real
OpenMCBase::stdev(const double & mean, const double & sum_sq, unsigned int realizations) const
{
  return realizations > 1
             ? std::sqrt(std::max(0.0, (sum_sq / realizations - mean * mean) / (realizations - 1)))
             : 0.0;
}

Real
OpenMCBase::relerr(const double & mean, const double & sum_sq, unsigned int realizations) const
{
  return mean > 0.0 ? stdev(mean, sum_sq, realizations) / mean : 0.0;
}

#endif
